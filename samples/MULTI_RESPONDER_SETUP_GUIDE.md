# Multi-Responder UWB Distance Measurement Setup Guide

This guide provides step-by-step instructions for setting up and using the multi-responder UWB distance measurement system with DWM3001CDK boards.

## Overview

The system consists of:
- **1 Initiator** (Example 07): Sends ranging requests and displays distances
- **1-4 Responders** (Example 08): Respond to ranging requests and calculate distances

## Hardware Requirements

- 2-5 DWM3001CDK boards
- USB cables for programming and power
- Computer with terminal software (minicom, PuTTY, etc.)
- Zephyr SDK and West build system installed

## System Architecture

```
┌─────────────────┐
│   Initiator     │
│     (ID: 1)     │ ──┐
│   Example 07    │   │
└─────────────────┘   │
                      │
┌─────────────────┐   │
│  Responder 1    │   │
│     (ID: 2)     │ ──┤
│   Example 08    │   │
└─────────────────┘   │
                      │
┌─────────────────┐   │
│  Responder 2    │   │
│     (ID: 3)     │ ──┤
│   Example 08    │   │
└─────────────────┘   │
                      │
┌─────────────────┐   │
│  Responder 3    │   │
│     (ID: 4)     │ ──┤
│   Example 08    │   │
└─────────────────┘   │
                      │
┌─────────────────┐   │
│  Responder 4    │   │
│     (ID: 5)     │ ──┘
│   Example 08    │
└─────────────────┘
```

## Quick Start

### Step 1: Prepare the Initiator

1. **Navigate to initiator directory:**
   ```bash
   cd uwb/sit/samples/07_sit_multi_responder_initiator
   ```

2. **Build and flash:**
   ```bash
   west build -b qorvo_dwm3001cdk
   west flash
   ```

3. **Label the board:** Mark this board as "INITIATOR" or "ID:1"

### Step 2: Prepare the Responders

For each responder board, repeat these steps:

1. **Navigate to responder directory:**
   ```bash
   cd uwb/sit/samples/08_sit_multi_responder_node
   ```

2. **Configure node ID:**
   Edit `src/main.c` and change the node ID:
   ```c
   // For first responder
   #define THIS_RESPONDER_NODE_ID 2
   
   // For second responder
   #define THIS_RESPONDER_NODE_ID 3
   
   // For third responder  
   #define THIS_RESPONDER_NODE_ID 4
   
   // For fourth responder
   #define THIS_RESPONDER_NODE_ID 5
   ```

3. **Build and flash:**
   ```bash
   west build -b qorvo_dwm3001cdk
   west flash
   ```

4. **Label the board:** Mark each board with its node ID (e.g., "RESP-2", "RESP-3", etc.)

### Step 3: Test the System

1. **Power on all boards**
2. **Connect to initiator via serial terminal:**
   ```bash
   minicom -D /dev/ttyACM0 -b 115200
   ```
   (Replace `/dev/ttyACM0` with your actual device path)

3. **Observe output:** You should see distance measurements appearing on the terminal

## Detailed Setup Instructions

### Configuring Multiple Responders

#### Method 1: Individual Configuration (Recommended)

For each responder board:

1. **Connect the board to your computer**
2. **Edit the configuration:**
   ```bash
   cd uwb/sit/samples/08_sit_multi_responder_node
   nano src/main.c  # or your preferred editor
   ```

3. **Change the node ID:**
   ```c
   #define THIS_RESPONDER_NODE_ID X  // Replace X with 2, 3, 4, or 5
   ```

4. **Build and flash:**
   ```bash
   west build -b qorvo_dwm3001cdk
   west flash
   ```

5. **Test individual responder:**
   ```bash
   minicom -D /dev/ttyACM0 -b 115200
   ```
   Verify the node displays correct ID in startup message

#### Method 2: Batch Configuration

If you have multiple development setups:

1. **Create separate build directories:**
   ```bash
   cd uwb/sit/samples/08_sit_multi_responder_node
   mkdir -p builds/node2 builds/node3 builds/node4 builds/node5
   ```

2. **Create configuration files for each node:**
   ```bash
   # Copy and modify main.c for each node
   cp src/main.c src/main_node2.c
   cp src/main.c src/main_node3.c
   # ... etc
   ```

3. **Edit each file with appropriate node ID**

4. **Build each configuration separately**

### Terminal Connection Setup

#### Linux/macOS:
```bash
# Find your device
ls /dev/ttyACM*

# Connect with minicom
minicom -D /dev/ttyACM0 -b 115200

# Or with screen
screen /dev/ttyACM0 115200
```

#### Windows:
- Use PuTTY or similar terminal software
- Set baud rate to 115200
- Find COM port in Device Manager

### Expected Output

#### Initiator Output:
```
Starting MULTI-RESPONDER DS-TWR Initiator v1.0
==========================================
Initiator ID: 1
Number of responders: 4
Responder IDs: 2 3 4 5
==========================================

SIT initialization successful
Starting ranging cycle...

✓ Responder 2: 12.45 m
✓ Responder 3: 8.23 m
✗ Responder 4: No response
✓ Responder 5: 15.67 m

=== Distance Measurements ===
Responder 1 (ID:2): 12.45 m
Responder 2 (ID:3):  8.23 m
Responder 3 (ID:4): NO RESPONSE
Responder 4 (ID:5): 15.67 m
=============================
```

#### Responder Output:
```
Starting MULTI-RESPONDER DS-TWR Node v1.0
==========================================

=== RESPONDER NODE INFO ===
Node ID: 2
Initiator ID: 1
===========================

SIT initialization successful
Responder node 2 ready and listening...

Node 2 -> Initiator: 12.45 m (seq: 15)
Node 2 -> Initiator: 12.47 m (seq: 47)
Stats - Polls: 20, Success: 18, Failed: 2, Rate: 90%
```

## Troubleshooting

### Common Issues

#### 1. No Response from Responders
**Problem:** Initiator shows "No response" for all responders

**Solutions:**
- Verify responder boards are powered on and running
- Check that responder node IDs are configured correctly (2, 3, 4, 5)
- Ensure all devices are within UWB range (~100m line-of-sight)
- Check for UWB interference from other devices

#### 2. Wrong Node ID
**Problem:** Responder shows wrong node ID in startup message

**Solutions:**
- Verify `THIS_RESPONDER_NODE_ID` is set correctly in `src/main.c`
- Rebuild and reflash the responder firmware
- Check that you're editing the correct file

#### 3. Build Errors
**Problem:** Compilation fails with errors

**Solutions:**
- Ensure Zephyr SDK is properly installed
- Verify you're in the correct directory
- Check that all SIT library dependencies are available
- Try cleaning the build: `west build -t clean`

#### 4. Serial Connection Issues
**Problem:** Cannot connect to device via serial

**Solutions:**
- Check device permissions: `sudo chmod 666 /dev/ttyACM0`
- Verify correct device path: `ls /dev/ttyACM*`
- Try different USB port or cable
- Close other applications using the serial port

#### 5. Inconsistent Distance Measurements
**Problem:** Distance values vary significantly or seem incorrect

**Solutions:**
- Ensure clear line-of-sight between devices
- Check for moving objects in measurement area
- Verify stable power supply to all devices
- Calibrate antenna delays if needed

### Debugging Steps

1. **Check Individual Components:**
   - Test each responder individually with a single responder setup
   - Verify initiator works with known good responder

2. **Monitor System Status:**
   - Check LED indicators on each board
   - Monitor serial output for error messages
   - Verify timing parameters are appropriate

3. **Environmental Factors:**
   - Test in different locations
   - Check for interference sources
   - Verify antenna positioning

## Advanced Configuration

### Customizing Node IDs

To use different node ID ranges:

1. **In initiator (`07_sit_multi_responder_initiator/src/main.c`):**
   ```c
   static uint8_t responder_ids[NUM_RESPONDERS] = {10, 11, 12, 13};
   ```

2. **In each responder (`08_sit_multi_responder_node/src/main.c`):**
   ```c
   #define THIS_RESPONDER_NODE_ID 10  // Match array above
   ```

### Adjusting Timing Parameters

For different performance requirements:

1. **Faster Updates (in initiator):**
   ```c
   #define RNG_DELAY_MS 50     // Reduce delay between responders
   #define CYCLE_DELAY_MS 500  // Reduce cycle delay
   ```

2. **Longer Range (in both initiator and responders):**
   ```c
   #define RESP_RX_TIMEOUT_UUS_T 2000  // Increase timeout
   ```

### Adding More Responders

To support more than 4 responders:

1. **In initiator:**
   ```c
   #define NUM_RESPONDERS 8
   static uint8_t responder_ids[NUM_RESPONDERS] = {2, 3, 4, 5, 6, 7, 8, 9};
   ```

2. **Configure additional responder boards with IDs 6, 7, 8, 9**

## Performance Characteristics

- **Update Rate:** ~1 Hz complete cycle (all responders)
- **Individual Responder Rate:** ~4 Hz per responder
- **Accuracy:** ±10cm typical under good conditions
- **Range:** Up to 100m line-of-sight
- **Power Consumption:** ~100mA average per device

## Safety and Compliance

- UWB operation complies with FCC Part 15 regulations
- No safety hazards under normal operation
- Avoid operating near sensitive electronics
- Follow local regulations for UWB usage

## Support

For issues not covered in this guide:
1. Check the individual README files in each example directory
2. Review the SIT library documentation
3. Consult Qorvo DWM3001CDK documentation
4. Check Zephyr RTOS documentation for build system issues

## Appendix

### File Structure
```
uwb/sit/samples/
├── 07_sit_multi_responder_initiator/
│   ├── src/main.c
│   ├── CMakeLists.txt
│   ├── prj.conf
│   └── README.md
├── 08_sit_multi_responder_node/
│   ├── src/main.c
│   ├── CMakeLists.txt
│   ├── prj.conf
│   └── README.md
└── MULTI_RESPONDER_SETUP_GUIDE.md
```

### Quick Reference Commands

```bash
# Build initiator
cd uwb/sit/samples/07_sit_multi_responder_initiator
west build -b qorvo_dwm3001cdk && west flash

# Build responder (edit node ID first!)
cd uwb/sit/samples/08_sit_multi_responder_node
west build -b qorvo_dwm3001cdk && west flash

# Connect to serial terminal
minicom -D /dev/ttyACM0 -b 115200
```
