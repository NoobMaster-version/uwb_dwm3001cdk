/*! ----------------------------------------------------------------------------
 *  @file    main.c
 *  @brief   Single-sided two-way ranging (SS TWR) initiator example code
 *
 *           This is a simple code example which acts as the initiator in a SS TWR distance measurement exchange. This application sends a "poll"
 *           frame (recording the TX time-stamp of the poll), after which it waits for a "response" message from the "DS TWR responder" example
 *           code (companion to this application) to complete the exchange. The response message contains the remote responder's time-stamps of poll
 *           RX, and response TX. With this data and the local time-stamps, (of poll TX and response RX), this example application works out a value
 *           for the time-of-flight over-the-air and, thus, the estimated distance between the two devices.
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <deca_probe_interface.h>
#include <deca_device_api.h>
#include <deca_regs.h>
#include <deca_spi.h>
#include <port.h>
#include <shared_defines.h>
#include <sit_led/sit_led.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Example application name */
#define APP_NAME "SS TWR INIT v1.0"

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,               /* Channel number. */
    DWT_PLEN_128,   /* Preamble length. Used in TX only. */
    DWT_PAC8,       /* Preamble acquisition chunk size. Used in RX only. */
    9,              /* TX preamble code. Used in TX only. */
    9,              /* RX preamble code. Used in RX only. */
    DWT_SFD_DW_8,   /* 0 to use standard 8 symbol SFD. */
    DWT_BR_6M8,     /* Data rate. */
    DWT_PHRMODE_STD,/* PHY header mode. */
    DWT_PHRRATE_STD,/* PHY header rate. */
    (129 + 8 - 8),  /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,  /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0      /* PDOA mode off */
};

/* Inter-ranging delay period, in milliseconds. */
#define RNG_DELAY_MS 1000

/* Default antenna delay values for 64 MHz PRF. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Length of the common part of the message (up to and including the function code, see NOTE 3 below). */
#define ALL_MSG_COMMON_LEN 10
/* Indexes to access some of the fields in the frames defined above. */
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4

/* Buffer to store received response message.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
#define RX_BUF_LEN 20
static uint8_t rx_buffer[RX_BUF_LEN];

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;

/* Time-stamps of frames transmission/reception. */
static uint64_t poll_tx_ts;
static uint64_t resp_rx_ts;

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. */
extern dwt_txconfig_t txconfig_options;

int main(void)
{
    uint32_t frame_len;

    /* Display application name on console. */
    printk("Starting %s\n", APP_NAME);

    /* Initialize LED subsystem */
    sit_led_init();

    /* Configure SPI rate, DW3000 supports up to 38 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC();

    /* Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC) */
    k_sleep(K_MSEC(2));

    /* Need to make sure DW IC is in IDLE_RC before proceeding */
    while (!dwt_checkidlerc()) {
        k_yield();
    }

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) {
        printk("INIT FAILED\n");
        sit_set_led(2, 1);  // Error LED
        return 1;
    }

    /* Configure DW IC. */
    if (dwt_configure(&config)) {
        printk("CONFIG FAILED\n");
        sit_set_led(2, 1);  // Error LED
        return 1;
    }

    /* Configure the TX spectrum parameters (power, PG delay and PG count) */
    dwt_configuretxrf(&txconfig_options);

    /* Apply default antenna delay values. */
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    /* Set expected response's timeout. */
    dwt_setrxtimeout(0);
    dwt_setpreambledetecttimeout(0);

    /* Set response delay time */
    dwt_setrxaftertxdelay(0);

    sit_set_led(1, 1);  // Success LED
    printk("Initialization successful\n");

    /* Loop forever initiating ranging exchanges. */
    while (1) {
        /* Write frame data to DW IC and prepare transmission. */
        uint8_t tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0x21};
        dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0); /* Zero offset in TX buffer. */
        dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1); /* Zero offset in TX buffer, ranging. */

        /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent. */
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

        /* Poll for reception of a frame or error/timeout. */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
            k_yield();
        }

        /* Clear good RX frame event and TX frame sent in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD | SYS_STATUS_ALL_TX);

        if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) {
            /* A frame has been received, read it into the local buffer. */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;
            if (frame_len <= RX_BUF_LEN) {
                dwt_readrxdata(rx_buffer, frame_len, 0);

                /* Get timestamps */
                poll_tx_ts = get_tx_timestamp_u64();
                resp_rx_ts = get_rx_timestamp_u64();

                /* Compute time of flight and distance. */
                int32_t rtd_init = resp_rx_ts - poll_tx_ts;
                uint32_t resp_tx_ts = (uint32_t)(rx_buffer[RESP_MSG_RESP_TX_TS_IDX] +
                                              (rx_buffer[RESP_MSG_RESP_TX_TS_IDX + 1] << 8) +
                                              (rx_buffer[RESP_MSG_RESP_TX_TS_IDX + 2] << 16) +
                                              (rx_buffer[RESP_MSG_RESP_TX_TS_IDX + 3] << 24));
                uint32_t poll_rx_ts = (uint32_t)(rx_buffer[RESP_MSG_POLL_RX_TS_IDX] +
                                              (rx_buffer[RESP_MSG_POLL_RX_TS_IDX + 1] << 8) +
                                              (rx_buffer[RESP_MSG_POLL_RX_TS_IDX + 2] << 16) +
                                              (rx_buffer[RESP_MSG_POLL_RX_TS_IDX + 3] << 24));

                double tof = ((rtd_init - (resp_tx_ts - poll_rx_ts)) / 2.0) * DWT_TIME_UNITS;
                double distance = tof * SPEED_OF_LIGHT;

                printk("Distance: %3.2f m\n", distance);
                sit_toggle_led(0);  // Toggle LED0 for activity indication
            }
        }
        else {
            printk("Error or timeout\n");
            sit_toggle_led(2);  // Toggle LED2 for error indication
        }

        /* Execute a delay between ranging exchanges. */
        k_sleep(K_MSEC(RNG_DELAY_MS));
    }

    return 0;
}
