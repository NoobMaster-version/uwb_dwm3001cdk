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

/* Node configuration */
uint8_t this_initiator_node_id  = 1;
uint8_t responder_node_ids[4]   = {2, 3, 4, 5};  /* IDs of the four responder devices */
uint8_t current_responder_idx   = 0;             /* Index to alternate between responders */

/* Store distances for each responder */
double distances[4] = {0.0, 0.0, 0.0, 0.0};
bool valid_distances[4] = {false, false, false, false};

/* Default antenna delay values for 64 MHz PRF */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Timing configurations */
#define CPU_PROCESSING_TIME 400
#define POLL_TX_TO_RESP_RX_DLY_UUS_T (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TO_FINAL_TX_DLY_UUS_T (350 + CPU_PROCESSING_TIME)
#define RESP_RX_TIMEOUT_UUS_T 1150
#define PRE_TIMEOUT 5
#define RANGING_DELAY_MS 50

int main(void) {
    /* Initialize LEDs */
    sit_led_init();

    /* Initialize UWB */
    int init_ok = sit_init();
    if(init_ok < 0) {
        sit_set_led(2, 0);
    } else {
        sit_set_led(1, 0);
    }

    LOG_INF("=== UWB Distance Measurement ===");
    LOG_INF("Initiator Node ID: %d", this_initiator_node_id);
    LOG_INF("Measuring distances to responders: 2, 3, 4, 5");
    LOG_INF("=====================================");

    uint8_t frame_sequenz = 0;

    while (1) {
        uint8_t responder_node_id = responder_node_ids[current_responder_idx];

        sit_set_rx_tx_delay_and_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS_T, RESP_RX_TIMEOUT_UUS_T);
        sit_set_preamble_detection_timeout(PRE_TIMEOUT);

        /* Send poll message */
        msg_simple_t twr_poll = {twr_1_poll, frame_sequenz, this_initiator_node_id, responder_node_id, 0};
        sit_start_poll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));

        /* Wait for response */
        msg_simple_t rx_resp_msg;
        msg_id_t msg_id = ds_twr_2_resp;

        if(sit_check_msg_id(msg_id, &rx_resp_msg)) {
            uint64_t poll_tx_ts = get_tx_timestamp_u64();
            uint64_t resp_rx_ts = get_rx_timestamp_u64();

            uint32_t final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS_T * UUS_TO_DWT_TIME)) >> 8;
            uint64_t final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

            /* Send final message */
            msg_ds_twr_final_t final_msg = {
                ds_twr_3_final,
                rx_resp_msg.header.sequence,
                rx_resp_msg.header.dest,
                rx_resp_msg.header.source,
                (uint32_t)poll_tx_ts,
                (uint32_t)resp_rx_ts,
                (uint32_t)final_tx_ts,
                0
            };

            if (sit_send_at((uint8_t*)&final_msg, sizeof(msg_ds_twr_final_t), final_tx_time)) {
                /* Calculate distance */
                uint32_t poll_tx_ts_32 = (uint32_t)poll_tx_ts;
                uint32_t resp_rx_ts_32 = (uint32_t)resp_rx_ts;
                double tof = ((double)(resp_rx_ts_32 - poll_tx_ts_32)) * DWT_TIME_UNITS / 2;

                /* Store distance */
                int resp_idx = current_responder_idx;
                valid_distances[resp_idx] = true;
                distances[resp_idx] = tof * SPEED_OF_LIGHT;

                /* Display distances */
                LOG_INF("Distance Measurements:");
                LOG_INF("d1: %.2fm, d2: %.2fm, d3: %.2fm, d4: %.2fm",
                       distances[0], distances[1], distances[2], distances[3]);
            }
        } else {
            valid_distances[current_responder_idx] = false;
            distances[current_responder_idx] = 0.0;
        }

        /* Move to next responder */
        current_responder_idx = (current_responder_idx + 1) % 4;
        frame_sequenz++;

        /* Small delay between measurements */
        k_msleep(RANGING_DELAY_MS);
    }
    return 0;
}
