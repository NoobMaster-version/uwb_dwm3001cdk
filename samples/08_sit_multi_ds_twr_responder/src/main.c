/*
 * Copyright (c) 2023 Sven Hoyer
 * 
 * Multi-responder DS-TWR responder example code. Each responder listens for
 * poll messages with its specific ID and responds accordingly.
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
#define APP_NAME "MULTI-RESPONDER DS-TWR RESPONDER v1.0"

/* Timing parameters */
#define POLL_RX_TO_RESP_TX_DLY_UUS 900
#define RESP_TX_TO_FINAL_RX_DLY_UUS 600
#define FINAL_RX_TIMEOUT_UUS 1200
#define PRE_TIMEOUT 5

/* Default antenna delay values for 64 MHz PRF */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* CONFIGURE THIS: Set responder ID (100-103) */
#define RESPONDER_ID 100

void main(void) {
    LOG_INF(APP_NAME);
    LOG_INF("===================");
    LOG_INF("Responder ID: %d", RESPONDER_ID);

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

    // Configure device settings
    device_settings.state = measurement;  // Always in measurement mode
    device_settings.deviceID = RESPONDER_ID;

    while (true) {
        // Listen for incoming poll messages
        sit_receive_now(0, 0);
        
        msg_simple_t rx_poll_msg;
        msg_id_t msg_id = twr_1_poll;

        if (sit_check_msg_id(msg_id, &rx_poll_msg)) {
            // Check if this poll is for us
            if (rx_poll_msg.header.dest == device_settings.deviceID) {
                LOG_INF("Received poll for this device (ID: %d)", device_settings.deviceID);
                
                // Get poll reception timestamp
                uint64_t poll_rx_ts = get_rx_timestamp_u64();

                // Calculate response transmission time
                uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

                // Prepare and send response message
                msg_simple_t msg_ds_poll_resp = {
                    {ds_twr_2_resp,
                     rx_poll_msg.header.sequence,
                     device_settings.deviceID,
                     rx_poll_msg.header.source},
                    0
                };

                // Configure for receiving final message
                sit_set_rx_after_tx_delay(RESP_TX_TO_FINAL_RX_DLY_UUS);
                sit_set_rx_timeout(FINAL_RX_TIMEOUT_UUS);
                sit_set_preamble_detection_timeout(PRE_TIMEOUT);

                // Send response and wait for final message
                if (!sit_send_at_with_response((uint8_t*)&msg_ds_poll_resp, 
                                             sizeof(msg_simple_t), 
                                             resp_tx_time)) {
                    LOG_WRN("Failed to send response");
                    continue;
                }

                // Process final message
                msg_ds_twr_final_t rx_final_msg;
                msg_id = ds_twr_3_final;

                if (sit_check_ds_final_msg_id(msg_id, &rx_final_msg)) {
                    // Get response transmission and final reception timestamps
                    uint64_t resp_tx_ts = get_tx_timestamp_u64();
                    uint64_t final_rx_ts = get_rx_timestamp_u64();

                    // Extract timestamps from final message
                    uint32_t poll_tx_ts = rx_final_msg.poll_tx_ts;
                    uint32_t resp_rx_ts = rx_final_msg.resp_rx_ts;
                    uint32_t final_tx_ts = rx_final_msg.final_tx_ts;

                    // Convert timestamps to 32-bit
                    uint32_t poll_rx_ts_32 = (uint32_t)poll_rx_ts;
                    uint32_t resp_tx_ts_32 = (uint32_t)resp_tx_ts;
                    uint32_t final_rx_ts_32 = (uint32_t)final_rx_ts;

                    // Calculate time of flight
                    double Ra = (double)(resp_rx_ts - poll_tx_ts);
                    double Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);
                    double Da = (double)(final_tx_ts - resp_rx_ts);
                    double Db = (double)(resp_tx_ts_32 - poll_rx_ts_32);

                    int64_t tof_dtu = (int64_t)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));

                    double tof = tof_dtu * DWT_TIME_UNITS;
                    double distance = tof * SPEED_OF_LIGHT;

                    LOG_INF("Distance calculated: %.2f m", distance);
                    sit_toggle_led(0);  // Toggle LED to indicate successful measurement
                } else {
                    LOG_WRN("Failed to receive final message");
                    dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
                }
            } else {
                LOG_DBG("Received poll for different device: %d", rx_poll_msg.header.dest);
            }
        }

        k_msleep(10);  // Small delay to prevent CPU overload
    }
}