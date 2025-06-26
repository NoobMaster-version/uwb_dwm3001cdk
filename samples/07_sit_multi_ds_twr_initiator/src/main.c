/*
 * Copyright (c) 2023 Sven Hoyer
 * 
 * Multi-responder DS-TWR initiator example code. This code performs 
 * distance measurements with multiple responders in sequence.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <deca_device_api.h>
#include <deca_probe_interface.h>

#include <sit/sit.h>
#include <sit/sit_device.h>
#include <sit/sit_distance.h>
#include <sit/sit_config.h>
#include <sit/sit_utils.h>
#include <sit_led/sit_led.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Example application name */
#define APP_NAME "MULTI-RESPONDER DS-TWR INITIATOR v1.0"

/* Default antenna delay values for 64 MHz PRF */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Timing parameters */
#define CPU_PROCESSING_TIME 400
#define POLL_TX_TO_RESP_RX_DLY_UUS (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TO_FINAL_TX_DLY_UUS (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TIMEOUT_UUS 1150
#define PRE_TIMEOUT 5

/* Number of responders to poll */
#define NUM_RESPONDERS 4
#define BASE_RESPONDER_ID 100  // Responders will be numbered 100, 101, 102, 103

/* Function to perform distance measurement with a single responder */
static bool measure_distance_to_responder(uint8_t responder_id, uint8_t sequence) {
    // Configure timing parameters
    sit_set_rx_after_tx_delay(POLL_TX_TO_RESP_RX_DLY_UUS);
    sit_set_rx_timeout(RESP_RX_TIMEOUT_UUS + 2000);
    sit_set_preamble_detection_timeout(PRE_TIMEOUT + 200);

    // Send poll message
    msg_simple_t twr_poll = {
        {twr_1_poll, sequence, device_settings.deviceID, responder_id}, 
        0
    };
    
    sit_start_poll((uint8_t*)&twr_poll, sizeof(twr_poll));

    // Wait for response
    msg_simple_t rx_resp_msg;
    msg_id_t msg_id = ds_twr_2_resp;

    if (!sit_check_msg_id(msg_id, &rx_resp_msg)) {
        LOG_WRN("No response from responder %d", responder_id);
        dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        return false;
    }

    // Get timestamps
    uint64_t poll_tx_ts = get_tx_timestamp_u64();
    uint64_t resp_rx_ts = get_rx_timestamp_u64();
    
    // Calculate final message transmission time
    uint32_t final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
    uint64_t final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

    // Prepare and send final message
    msg_ds_twr_final_t final_msg = {
        {ds_twr_3_final, rx_resp_msg.header.sequence, 
         rx_resp_msg.header.dest, rx_resp_msg.header.source},
        (uint32_t)poll_tx_ts,
        (uint32_t)resp_rx_ts,
        (uint32_t)final_tx_ts,
        0
    };

    if (sit_send_at((uint8_t*)&final_msg, sizeof(final_msg), final_tx_time)) {
        LOG_INF("Distance measurement cycle completed with responder %d", responder_id);
        return true;
    }
    
    LOG_WRN("Failed to send final message to responder %d", responder_id);
    return false;
}

void main(void) {
    LOG_INF(APP_NAME);
    LOG_INF("===================");

    // Initialize LEDs for status indication
    sit_led_init();

    // Initialize UWB device
    int init_ok = sit_init();
    if (init_ok < 0) {
        LOG_ERR("Device initialization failed");
        sit_set_led(2, 1);  // Error indication
        return;
    }

    // Set antenna delays
    set_antenna_delay(RX_ANT_DLY, TX_ANT_DLY);
    
    sit_set_led(1, 1);  // Success indication

    // Configure device as initiator
    device_settings.deviceID = 1;     // Initiator ID
    device_settings.state = measurement;
    device_settings.responder = BASE_RESPONDER_ID + NUM_RESPONDERS - 1;
    
    uint8_t sequence = 0;
    
    LOG_INF("Starting measurements with %d responders", NUM_RESPONDERS);
    LOG_INF("Responder IDs: %d to %d", BASE_RESPONDER_ID, device_settings.responder);
    
    while (true) {
        // Measure distance to each responder in sequence
        for (uint8_t i = 0; i < NUM_RESPONDERS; i++) {
            uint8_t responder_id = BASE_RESPONDER_ID + i;
            
            if (measure_distance_to_responder(responder_id, sequence)) {
                sit_toggle_led(0);  // Toggle LED to indicate successful measurement
            }
            
            // Small delay between measurements
            k_msleep(50);
        }

        sequence++;
        
        // Delay before next round of measurements
        k_msleep(100);
    }
}