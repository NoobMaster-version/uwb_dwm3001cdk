# Multi-Responder DS-TWR Node (Responder)

This example demonstrates a Double-Sided Two-Way Ranging (DS-TWR) responder node that works with a multi-responder initiator system. Each responder can be configured with a unique ID (2-5) to participate in distance measurements.

## Overview

The responder listens for poll messages from the initiator, responds when its ID matches, and participates in the DS-TWR protocol for accurate distance measurement. This code is designed to work specifically with the multi-responder initiator (example 07).

## Features

- **Configurable node ID**: Set as responder ID 2, 3, 4, or 5
- **DS-TWR protocol support**: High-accuracy distance measurement
- **LED status indicators**: Visual feedback for system status
- **Selective response**: Only processes messages intended for its ID
- **Error handling**: Manages communication failures gracefully

## Configuration

### Important: Setting Node ID

Each responder must be configured with a unique ID. Modify this line in `src/main.c`:

```c
uint8_t responder_node_id = 2;  // Change to 2, 3, 4, or 5
```

Valid responder IDs:
- Responder 1: ID = 2
- Responder 2: ID = 3
- Responder 3: ID = 4
- Responder 4: ID = 5

## Hardware Requirements

- 1x DWM3001CDK board
- Compatible with multi-responder initiator (example 07)
- UWB communication range: typically <100m line-of-sight

## Building and Flashing

```bash
cd uwb/sit/samples/08_sit_multi_responder_node
# Edit src/main.c to set your desired responder_node_id (2-5)
west build -b qorvo_dwm3001cdk
west flash
```

## Setup Instructions

1. Choose a unique responder ID (2-5)
2. Edit `responder_node_id` in `src/main.c`
3. Build and flash the firmware
4. Label the board with its ID for easy identification
5. Repeat for other responder boards using different IDs

## Usage

1. Power on the responder board
2. Check initialization message in terminal (115200 baud)
3. Verify LED status indicators
4. Ensure initiator is operating (example 07)
5. Monitor for ranging activity

## Terminal Output

The responder displays:
```
SIMPLE DS-TWR Responder EXAMPLE (ID configurable from 2-5)
==================
Responder ID: X
```

Where X is your configured responder ID (2-5)

## LED Indicators

- **LED 1**: System ready (solid on after successful initialization)
- **LED 2**: Error indicator (solid on if initialization fails)

## Troubleshooting

### No Communication with Initiator
- Verify correct responder ID configuration
- Check that initiator is powered and running
- Ensure devices are within range
- Check for physical obstacles

### LED Status Issues
- LED 1 off: Initialization problem
- LED 2 on: System error detected
- Both LEDs off: Check power supply

### Build Problems
- Verify Zephyr SDK installation
- Check board configuration
- Ensure correct toolchain setup

## Technical Details

### Timing Parameters
- Poll-to-Response delay: 900μs
- Response-to-Final delay: 600μs
- Final message timeout: 1200μs
- Processing delay: 80ms between operations

### Message Flow
1. Listen for poll message
2. Check if message is for this responder
3. Send response message
4. Wait for final message
5. Process ranging data
6. Repeat cycle

## Known Limitations

- Fixed responder ID system (2-5 only)
- Single initiator support
- Sequential ranging only
- 80ms minimum delay between measurements

## Integration Notes

This responder code is specifically designed to work with the multi-responder initiator (example 07). Ensure all responders in your system have unique IDs and that the initiator is properly configured to communicate with all deployed responders.