# Multi-Responder DS-TWR Initiator

This example demonstrates a Double-Sided Two-Way Ranging (DS-TWR) initiator that communicates with four responder nodes to measure distances simultaneously.

## Overview

The initiator cycles through 4 fixed responder nodes (IDs: 2, 3, 4, 5), performing distance measurements with each one sequentially. The system displays real-time distance measurements in a clean, easy-to-read format.

## Features

- **Fixed 4-responder support**: Communicates with responder nodes IDs 2-5
- **DS-TWR protocol**: Uses double-sided two-way ranging for accurate distance measurement
- **Clean display format**: Shows distances in an easy-to-read format (d1 - XXm, d2 - XXm, etc.)
- **LED status indicators**: Visual feedback for system status
- **Error handling**: Gracefully handles non-responsive responders

## Hardware Requirements

- 1x DWM3001CDK board (configured as initiator)
- 4x DWM3001CDK boards (configured as responders using example 08)
- All boards should be within UWB communication range (typically <100m line-of-sight)

## Building and Flashing

```bash
cd uwb/sit/samples/07_sit_multi_responder_initiator
west build -b qorvo_dwm3001cdk
west flash
```

## Usage

1. Program one DWM3001CDK board with this initiator code
2. Program 4 additional boards with the responder code (example 08)
3. Set unique IDs for each responder (2, 3, 4, and 5)
4. Power on all devices
5. Monitor the initiator's output via serial terminal (115200 baud)

## Terminal Output Format

The initiator displays distance measurements in a clean, simple format:

```
=== Distance Measurements ===
d1 - 2.34m, d2 - 1.56m, d3 - 3.45m, d4 - 2.78m
```

Where:
- d1: Distance to responder ID 2
- d2: Distance to responder ID 3
- d3: Distance to responder ID 4
- d4: Distance to responder ID 5

## LED Indicators

- **LED 1**: System ready (solid on after successful initialization)
- **LED 2**: Error indicator (solid on if initialization fails)

## Troubleshooting

### No Response from a Responder
- Verify the responder is powered on
- Check that the responder has the correct ID configured
- Ensure the responder is within range
- Check for any obstacles between devices

### Inconsistent Measurements
- Ensure clear line-of-sight between devices
- Check for interference from other UWB devices
- Verify stable power supply to all devices

### Build Issues
- Ensure correct Zephyr SDK installation
- Verify all SIT library dependencies are available
- Check that board configuration matches your hardware

## Technical Notes

### Timing Parameters
- Poll-to-Response delay: 350μs + processing time
- Response-to-Final delay: 350μs + processing time
- Response timeout: 1150μs
- Inter-measurement delay: 50ms

### Communication Flow
1. Send poll message to current responder
2. Wait for response message
3. Send final message with timing data
4. Calculate distance
5. Move to next responder
6. Display updated measurements
7. Repeat cycle

### Distance Calculation
Uses DS-TWR algorithm to compensate for clock drift between devices, providing more accurate measurements than single-sided TWR.

## Known Limitations

- Fixed to exactly 4 responders
- Sequential ranging (one responder at a time)
- 80ms delay between measurements
- Display updates may flicker in some terminal emulators