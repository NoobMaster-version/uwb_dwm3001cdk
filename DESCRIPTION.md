# UWB Sport Indoor Tracking (SIT) System

This document provides an overview of the Ultra-Wideband (UWB) ranging system designed for Sports Indoor Tracking (SIT). The system uses Decawave DW3000 UWB transceivers and implements Double-Sided Two-Way Ranging (DS-TWR) to accurately measure the distance between two devices: an **Initiator** and a **Responder**.

## System Architecture

The project is structured into a custom library `lib/sit` and example applications in `samples/`.

- **`lib/sit`**: A dedicated library that abstracts the complexities of the DW3000 driver and provides a streamlined API for ranging and device configuration.
- **`samples/`**: Contains example applications demonstrating the usage of the `lib/sit` library. The key examples are `05_sit_ds_twr_initator` and `06_sit_ds_twr_responder`, which work together to perform distance measurements.
- **`drivers/dw3000`**: The low-level driver for the DW3000 chip, providing the core functions to control the UWB transceiver.

## Ranging Protocol: Double-Sided Two-Way Ranging (DS-TWR)

The system uses DS-TWR to calculate the distance between the Initiator and the Responder. This method is robust against clock drift and provides high accuracy. The process involves an exchange of four messages.

### Message Sequence

1.  **Poll (Initiator -> Responder)**: The Initiator starts the exchange by sending a `poll` message and records its transmission timestamp (`poll_tx_ts`).

2.  **Response (Responder -> Initiator)**:
    - The Responder receives the `poll` message and records its reception timestamp (`poll_rx_ts`).
    - After a predefined delay, the Responder sends a `response` message back to the Initiator and records its transmission timestamp (`resp_tx_ts`).

3.  **Final (Initiator -> Responder)**:
    - The Initiator receives the `response` message and records its reception timestamp (`resp_rx_ts`).
    - The Initiator now has three timestamps. It sends a `final` message to the Responder that contains the `poll_tx_ts` and `resp_rx_ts`. It also records the transmission time of this final message (`final_tx_ts`).

4.  **Distance Calculation (Responder)**:
    - The Responder receives the `final` message. It now has all the necessary timestamps from both devices to calculate the time of flight (ToF).
    - The Responder records the reception time of the final message (`final_rx_ts`).

### Time of Flight (ToF) Calculation

The ToF is calculated using the timestamps recorded by both devices. The formula for symmetric DS-TWR is:

```
round_trip_1 = resp_rx_ts (Initiator) - poll_tx_ts (Initiator)
reply_time_1 = resp_tx_ts (Responder) - poll_rx_ts (Responder)

round_trip_2 = final_rx_ts (Responder) - resp_tx_ts (Responder)
reply_time_2 = final_tx_ts (Initiator) - resp_rx_ts (Initiator)

ToF = ( (round_trip_1 * round_trip_2) - (reply_time_1 * reply_time_2) ) / ( round_trip_1 + round_trip_2 + reply_time_1 + reply_time_2 )
```

The distance is then calculated as:

`Distance = ToF * SPEED_OF_LIGHT`

## Core Library: `lib/sit`

The `sit` library is the heart of the application, providing a high-level interface for the ranging process.

- **`sit.c`**: The main entry point for the SIT system. It contains the state machine for different measurement modes (SS-TWR, DS-TWR, calibration) and handles the overall application logic. It also integrates with a BLE module to transmit the final calculated distance.
- **`sit_config.c`**: Defines the runtime and build-time configurations for the DW3000 chip. This includes:
    - **UWB Parameters**: Channel (9), PRF (64 MHz), Preamble Length (128 symbols), Data Rate (6.8 Mbps), etc.
    - **Device Settings**: Device ID, role (Initiator/Responder), and antenna delay calibration values (`tx_ant_dly`, `rx_ant_dly`).
- **`sit_distance.c`**: Implements the functions that manage the UWB message exchange for ranging. It wraps lower-level driver functions into a more straightforward API (e.g., `sit_start_poll`, `sit_send_at`, `sit_check_msg`).
- **`sit_diagnostic.c`**: Provides functions for Non-Line-of-Sight (NLOS) detection. It analyzes the Channel Impulse Response (CIR) to estimate the First Path Signal Level (FSL) and Received Signal Strength Indicator (RSSI). A large difference between these values can indicate an obstructed path.
- **`sit_utils.c`**: Contains helper functions, primarily for reading the 40-bit TX and RX timestamps from the DW3000 registers.
- **`sit_device.c`**: A hardware abstraction layer that simplifies interaction with the DW3000 chip, including setting antenna delays and reading device status.

## How to Use

1.  **Flash the code**:
    - Flash the `05_sit_ds_twr_initator` application onto one DW3000-based device.
    - Flash the `06_sit_ds_twr_responder` application onto a second device.
2.  **Observe**: The devices will start the DS-TWR exchange. The Responder will calculate the distance and print it to the console/log. The `sit.c` file also includes functionality to send this data over BLE.

This system provides a solid foundation for building advanced indoor positioning and tracking applications.
