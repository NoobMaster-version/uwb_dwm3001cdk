/*! ----------------------------------------------------------------------------
 *  @file    ds_twr_initiator.c
 *  @brief   Double-sided two-way ranging (DS TWR) initiator example code
 *
 *           This is a simple code example which acts as the initiator in a DS TWR distance measurement exchange.
 *           This application cycles through a list of responders, performing a DS TWR exchange with each one in a
 *           dedicated time slot.
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include "deca_device_api.h"

#include <sit/sit.h>
#include <sit/sit_device.h>
#include <sit/sit_distance.h>
#include <sit/sit_config.h>
#include <sit/sit_utils.h>
#include <sit_led/sit_led.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Example application name */
#define APP_NAME "DS-TWR INITIATOR EXAMPLE\n"

/* List of responder device IDs to range with */
static const uint8_t responder_id_list[] = {2, 3, 4, 5};
#define NUM_RESPONDERS (sizeof(responder_id_list) / sizeof(responder_id_list[0]))

/* This is the initiator's ID. */
static uint8_t this_initiator_node_id = 1;

/* Time slot for each responder in milliseconds. */
#define TIME_SLOT_MS 25

/* Default antenna delay values for 64 MHz PRF. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* DS-TWR delays */
#define POLL_TX_TO_RESP_RX_DLY_UUS_T (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TO_FINAL_TX_DLY_UUS_T (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TIMEOUT_UUS_T 2000
#define PRE_TIMEOUT 5

int main(void)
{
    printk(APP_NAME);
    printk("==================\n");

    // INIT LED and let them Blink one Time to see Intitalion Finshed
    sit_led_init();

    int init_ok = sit_init();

    if (init_ok < 0)
    {
        sit_set_led(2, 0);
    }
    else
    {
        sit_set_led(1, 0);
    }

    uint8_t frame_sequenz = 0;
    uint8_t responder_idx = 0;

    while (1)
    {
        uint8_t current_responder_id = responder_id_list[responder_idx];

        sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS_T, RESP_RX_TIMEOUT_UUS_T);
        sit_set_preamble_detection_timeout(PRE_TIMEOUT);

        LOG_INF("Ranging with responder ID: %d", current_responder_id);
        msg_simple_t twr_poll = {{twr_1_poll, frame_sequenz, this_initiator_node_id, current_responder_id}, 0};
        sit_start_poll((uint8_t *)&twr_poll, (uint16_t)sizeof(twr_poll));

        msg_simple_t rx_resp_msg;
        msg_id_t msg_id = ds_twr_2_resp;

        if (sit_check_msg_id(msg_id, &rx_resp_msg))
        {
            uint64_t poll_tx_ts = get_tx_timestamp_u64();
            uint64_t resp_rx_ts = get_rx_timestamp_u64();

            uint32_t final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS_T * UUS_TO_DWT_TIME)) >> 8;
            uint64_t final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

            msg_ds_twr_final_t final_msg = {
                {ds_twr_3_final,
                 rx_resp_msg.header.sequence,
                 rx_resp_msg.header.dest,
                 rx_resp_msg.header.source},
                (uint32_t)poll_tx_ts,
                (uint32_t)resp_rx_ts,
                (uint32_t)final_tx_ts,
                0};

            bool ret = sit_send_at_with_response((uint8_t *)&final_msg, sizeof(msg_ds_twr_final_t), final_tx_time);

            if (ret == false)
            {
                LOG_WRN("Failed to send final message to responder %d", current_responder_id);
            }
            else
            {
                msg_ds_twr_resp_t rx_final_resp_msg;
                msg_id = ds_twr_4_final;
                if (sit_check_ds_resp_msg_id(msg_id, &rx_final_resp_msg))
                {
                    uint32_t poll_tx_ts_32 = final_msg.poll_tx_ts;
                    uint32_t resp_rx_ts_32 = final_msg.resp_rx_ts;
                    uint32_t final_tx_ts_32 = final_msg.final_tx_ts;

                    uint32_t poll_rx_ts_resp = rx_final_resp_msg.poll_rx_ts;
                    uint32_t resp_tx_ts_resp = rx_final_resp_msg.resp_tx_ts;
                    uint32_t final_rx_ts_resp = rx_final_resp_msg.final_rx_ts;

                    double Ra, Rb, Da, Db;
                    int64_t tof_dtu;
                    Ra = (double)(resp_rx_ts_32 - poll_tx_ts_32);
                    Rb = (double)(final_rx_ts_resp - resp_tx_ts_resp);
                    Da = (double)(final_tx_ts_32 - resp_rx_ts_32);
                    Db = (double)(resp_tx_ts_resp - poll_rx_ts_resp);
                    tof_dtu = (int64_t)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));

                    double tof = tof_dtu * DWT_TIME_UNITS;
                    double distance = tof * SPEED_OF_LIGHT;
                    LOG_INF("Distance to responder %d: %.2f m", current_responder_id, distance);
                }
                else
                {
                    LOG_WRN("Did not receive final response from responder %d", current_responder_id);
                    dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
                }
            }
        }
        else
        {
            LOG_WRN("Did not receive response from responder %d", current_responder_id);
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        }

        // Move to the next responder in the list
        responder_idx = (responder_idx + 1) % NUM_RESPONDERS;
        frame_sequenz++;
        k_msleep(TIME_SLOT_MS);
    }
    return 0;
}
