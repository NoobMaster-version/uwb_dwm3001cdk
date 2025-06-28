/**
 * Copyright (c) 2019 - Frederic Mes, RTLOC
 * Copyright (c) 2015 - Decawave Ltd, Dublin, Ireland.
 * Copyright (c) 2021 - Home Smart Mesh
 * Copyright (c) 2022 - Sven Hoyer
 *
 * This file is part of Zephyr-DWM1001.
 *
 *   Zephyr-DWM1001 is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Zephyr-DWM1001 is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Zephyr-DWM1001.  If not, see <https://www.gnu.org/licenses/>.
 *
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

#define APP_NAME "DS-TWR RESPONDER EXAMPLE\n"

/* This is the responder's ID. */
#define RESPONDER_ID 5

/* Default antenna delay values for 64 MHz PRF. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* DS-TWR delays */
#define POLL_RX_TO_RESP_TX_DLY_UUS_T 900
#define RESP_TX_TO_FINAL_RX_DLY_UUS_T 600
#define FINAL_RX_TIMEOUT_T 1200
#define PRE_TIMEOUT 5

int main(void)
{
    printk(APP_NAME);
    printk("==================\n");

    // INIT LED and let them Blink one Time to see Intitalion Finshed
    sit_led_init();

    int init_ok = 0;
    do
    {
        init_ok = sit_init();
    } while (init_ok > 1);

    if (init_ok < 0)
    {
        sit_set_led(2, 1);
    }
    else
    {
        sit_set_led(1, 1);
    }

    uint8_t frame_sequenz = 0;

    while (1)
    {
        sit_receive_now(0, 0);
        msg_simple_t rx_poll_msg;
        msg_id_t msg_id = twr_1_poll;
        if (sit_check_msg_id(msg_id, &rx_poll_msg) && (rx_poll_msg.header.dest == RESPONDER_ID))
        {
            uint64_t poll_rx_ts = get_rx_timestamp_u64();

            uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS_T * UUS_TO_DWT_TIME)) >> 8;

            msg_simple_t msg_ds_poll_resp = {
                {ds_twr_2_resp,
                 rx_poll_msg.header.sequence,
                 rx_poll_msg.header.dest,
                 rx_poll_msg.header.source},
                0};
            sit_set_rx_after_tx_delay(RESP_TX_TO_FINAL_RX_DLY_UUS_T);
            sit_set_rx_timeout(FINAL_RX_TIMEOUT_T);
            sit_set_preamble_detection_timeout(PRE_TIMEOUT);
            bool ret = sit_send_at_with_response((uint8_t *)&msg_ds_poll_resp, sizeof(msg_simple_t), resp_tx_time);
            if (ret == false)
            {
                LOG_WRN("Failed to send response to initiator");
                continue;
            }

            msg_ds_twr_final_t rx_ds_final_msg;
            msg_id = ds_twr_3_final;
            if (sit_check_ds_final_msg_id(msg_id, &rx_ds_final_msg))
            {
                uint64_t resp_tx_ts = get_tx_timestamp_u64();
                uint64_t final_rx_ts = get_rx_timestamp_u64();

                uint32_t final_resp_tx_time = (final_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS_T * UUS_TO_DWT_TIME)) >> 8;

                msg_ds_twr_resp_t final_resp_msg = {{(msg_id_t)ds_twr_4_final,
                                                     rx_ds_final_msg.header.sequence,
                                                     rx_ds_final_msg.header.dest,
                                                     rx_ds_final_msg.header.source},
                                                    (uint32_t)poll_rx_ts,
                                                    (uint32_t)resp_tx_ts,
                                                    (uint32_t)final_rx_ts,
                                                    0};

                ret = sit_send_at((uint8_t *)&final_resp_msg, sizeof(msg_ds_twr_resp_t), final_resp_tx_time);
                if (ret == false)
                {
                    LOG_WRN("Failed to send final response to initiator");
                }
            }
            else
            {
                LOG_WRN("Did not receive final message from initiator");
                dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            }
        }
        else
        {
            // Not our poll, just go back to listening
        }
        frame_sequenz++;
    }

    return 0;
}
