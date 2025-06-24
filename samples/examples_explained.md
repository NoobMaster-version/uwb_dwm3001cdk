# UWB Sample Applications

This directory contains various examples demonstrating different features and capabilities of the UWB (Ultra-Wideband) system with DWM3001 boards. Below is a detailed explanation of each example:

## 1. Hello LEDs (00_hello_leds)
- A basic example demonstrating LED control on the DWM3001 board
- Features:
  - Initializes all 4 LEDs on the board
  - Creates a running light pattern where LEDs light up sequentially
  - Demonstrates basic GPIO control and timing functions
  - Uses Zephyr's logging system for debug output
- Purpose: Serves as a basic hardware test and introduction to the board's LED control

## 2. SIT LED Example (01_sit_led_ex)
- A more structured example using the SIT (Sports Indoor Tracking) LED subsystem
- Features:
  - Uses the SIT LED API for LED control
  - Shows proper LED initialization and control methods
  - Demonstrates toggle functionality of LEDs
  - Creates an alternating pattern between different LEDs
- Purpose: Shows how to properly use the SIT LED subsystem in more complex applications

## 3. BLE Basic Example (02_sit_ble_example)
- Demonstrates basic Bluetooth Low Energy functionality
- Features:
  - Sets up a BLE peripheral device
  - Implements a custom service with read/write characteristics
  - Uses LED indicators for connection status
  - Handles BLE connections and disconnections
  - Implements basic data exchange over BLE
- Purpose: Introduction to BLE communication capabilities of the board

## 4. Single-Sided Two-Way Ranging Examples
### a. Initiator (03a_sit_ss_twr_initator)
- Implements the initiator role in SS-TWR (Single-Sided Two-Way Ranging)
- Features:
  - Sends poll messages to responder
  - Calculates distance based on response timing
  - Uses UWB radio for precise timing measurements
  - Displays calculated distances
- Purpose: Distance measurement from initiator's perspective

### b. Initiator with BLE Output (03b_sit_ss_twr_initator_ble_output)
- Combines SS-TWR initiator with BLE communication
- Features:
  - Same ranging functionality as basic initiator
  - Adds BLE output of measured distances
  - Automatic data transmission over BLE
- Purpose: Remote monitoring of distance measurements

### c. Initiator with BLE Input/Output (03c_sit_ss_twr_initator_ble_in-output)
- Enhanced version with bidirectional BLE communication
- Features:
  - Full SS-TWR functionality
  - BLE control of ranging process
  - Two-way data exchange over BLE
  - Configurable parameters via BLE
- Purpose: Interactive distance measurement system

## 5. Single-Sided TWR Responder (04_sit_ss_twr_responder)
- Implements the responder role in SS-TWR
- Features:
  - Listens for poll messages
  - Responds with precise timing information
  - Handles multiple ranging requests
  - LED indicators for activity
- Purpose: Companion to the initiator examples

## 6. Double-Sided TWR Examples
### a. Initiator (05_sit_ds_twr_initator)
- Implements DS-TWR (Double-Sided Two-Way Ranging) initiator
- Features:
  - More accurate ranging than SS-TWR
  - Additional message exchange for better precision
  - Enhanced error correction
  - Detailed timing calculations
- Purpose: High-precision distance measurement

### b. Responder (06_sit_ds_twr_responder)
- Implements DS-TWR responder functionality
- Features:
  - Handles advanced ranging protocol
  - Precise timing responses
  - Error handling and recovery
  - Status indicators
- Purpose: Companion to DS-TWR initiator

## 7. BLE Mesh Example (10_sit_ble_mesh)
- Demonstrates BLE mesh networking capabilities
- Features:
  - Mesh network formation
  - Node provisioning
  - Message routing
  - Network status indicators
  - Mesh configuration options
- Purpose: Creating scalable device networks

## 8. JSON Communication Example (20_sit_ble_json)
- Shows JSON data handling capabilities
- Features:
  - JSON message parsing
  - Data structure conversion
  - Error handling
  - Format validation
- Purpose: Structured data exchange between devices

## Common Features Across Examples
- Comprehensive error handling
- LED status indicators
- Debug logging
- Configuration options
- Hardware abstraction
- Power management consideration

## Building and Running
Each example can be built using:
```bash
west build -b qorvo_dwm3001cdk -- -DCONF_FILE=prj.conf
west flash
