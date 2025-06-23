# DWM3001CDK UWB Development Setup Guide

A comprehensive guide for setting up the development environment and running examples for the DWM3001CDK Ultra-Wideband (UWB) board.

## Prerequisites

### Hardware Required
- DWM3001CDK board
- USB cable (Type-A to Micro-B)
- Linux-based development system
- For distance measurement examples: two DWM3001CDK boards

### Software Prerequisites
- Python 3.8 or newer
- CMake 3.20 or newer
- Git
- Build tools

## System Setup

### 1. Install System Dependencies (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev
```

### 2. Install Python Dependencies
```bash
# Install west (Zephyr's meta-tool)
pip3 install --user west

# Add ~/.local/bin to PATH
echo 'export PATH=~/.local/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

## Project Setup

### 1. Create Workspace and Clone Repository
```bash
# Create and navigate to workspace directory
mkdir ~/workspace
cd ~/workspace

# Initialize workspace with west
west init -m https://github.com/NoobMaster-version/uwb_dwm3001cdk.git --mr main

# Update west dependencies
west update
```

### 2. Install Zephyr SDK
```bash
# Download SDK
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.1/zephyr-sdk-0.17.1_linux-x86_64.tar.gz

# Extract SDK
tar xvf zephyr-sdk-0.17.1_linux-x86_64.tar.gz

# Run setup script
cd zephyr-sdk-0.17.1
./setup.sh

# Add SDK to environment
echo 'export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.17.1' >> ~/.bashrc
```

### 3. Set Up Environment Variables
```bash
# Add these to your ~/.bashrc
export ZEPHYR_BASE=~/workspace/uwb/zephyr
source $ZEPHYR_BASE/zephyr-env.sh
```

### 4. Install Additional Dependencies
```bash
# Install Python dependencies
pip3 install --user pyelftools
pip3 install --user -r ~/workspace/uwb/zephyr/scripts/requirements.txt

# Setup udev rules for the board
sudo cp ~/workspace/uwb/zephyr/zephyr-env.sh /etc/udev/rules.d/99-jlink.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

## Building and Flashing Examples

### Basic LED Example (00_hello_leds)
```bash
# Navigate to example
cd ~/workspace/uwb/sit/samples/00_hello_leds

# Clean build directory
rm -rf build

# Build
west build -b qorvo_dwm3001cdk

# Flash
west flash
```

### Single-Sided TWR Example (03a_sit_ss_twr_initator)
```bash
# Build and flash initiator
cd ~/workspace/uwb/sit/samples/03a_sit_ss_twr_initator
west build -b qorvo_dwm3001cdk
west flash

# For the responder (on second board)
cd ~/workspace/uwb/sit/samples/04_sit_ss_twr_responder
west build -b qorvo_dwm3001cdk
west flash
```

### BLE Example (02_sit_ble_example)
```bash
cd ~/workspace/uwb/sit/samples/02_sit_ble_example
west build -b qorvo_dwm3001cdk
west flash
```

## Monitoring Serial Output

### Option 1: Using screen
```bash
sudo apt-get install screen
screen /dev/ttyACM0 115200
# To exit: Ctrl+A, then ':quit' and Enter
```

### Option 2: Using minicom
```bash
sudo apt-get install minicom
minicom -D /dev/ttyACM0 -b 115200
# To exit: Ctrl+A, then 'x'
```

### Option 3: Using tio
```bash
sudo apt-get install tio
tio /dev/ttyACM0 -b 115200
# To exit: Ctrl+C
```

## Common Issues and Solutions

### Permission Issues
If you encounter permission denied errors:
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and log back in for changes to take effect
```

### Build Issues
- Verify environment variables are set correctly
- Make sure all dependencies are installed
- Try cleaning and rebuilding:
```bash
rm -rf build
west build -b qorvo_dwm3001cdk
```

### Flash Issues
- Ensure board is properly connected
- Try unplugging and reconnecting the board
- Verify board detection:
```bash
lsusb
ls -l /dev/ttyACM*
```

## Example Types and Usage

1. **LED Examples**
   - `00_hello_leds`: Basic LED control
   - `01_sit_led_ex`: LED library usage

2. **Bluetooth Examples**
   - `02_sit_ble_example`: Basic BLE connectivity
   - `10_sit_ble_mesh`: BLE mesh networking
   - `20_sit_ble_json`: JSON over BLE

3. **Distance Measurement**
   - `03a_sit_ss_twr_initator`: Basic distance measurement (initiator)
   - `04_sit_ss_twr_responder`: Basic distance measurement (responder)
   - `05_sit_ds_twr_initator`: Advanced distance measurement (initiator)
   - `06_sit_ds_twr_responder`: Advanced distance measurement (responder)

## Additional Resources
- [Zephyr Project Documentation](https://docs.zephyrproject.org/)
- [DWM3001CDK Documentation](https://www.qorvo.com/products/p/DWM3001CDK)
- [Project GitHub Repository](https://github.com/NoobMaster-version/uwb_dwm3001cdk.git)

## Support
For issues and questions:
1. Check the [GitHub Issues](https://github.com/NoobMaster-version/uwb_dwm3001cdk/issues)
2. Create a new issue if needed
3. Include logs and build output when reporting problems