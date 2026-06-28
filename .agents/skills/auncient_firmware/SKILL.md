---
name: auncient-firmware-control
description: Enables agents to compile, flash, test, and monitor the OOK/LoRa dual-modulation firmware running on Heltec v4 ESP32-S3 boards.
---

# Auncient Firmware Control & Test Runner Instructions

This skill enables any agentic coding assistant to interact with and operate the dual-modulation Kermit-over-OOK firmware flashed onto the local ESP32-S3 devices.

## 1. Hardware Pin & Port Manifest
* **Device 1 Port**: `/dev/ttyACM0` (USB JTAG/Serial console)
* **Device 2 Port**: `/dev/ttyACM1` (USB JTAG/Serial console)
* **Onboard SX1262 Transceiver Pinouts**:
  * `MISO` = GPIO 11, `MOSI` = GPIO 10, `SCK` = GPIO 9, `CS` = GPIO 8
  * `RESET` = GPIO 12, `BUSY` = GPIO 13
  * `DIO2` (OOK Modulator Line) = GPIO 15
* **Safe Transmit Footprint**: Fixed to **$+4\text{ dBm}$** power.

## 2. Compilation and Flashing

### Environment Export
Always export the ESP-IDF path before invoking build tools:
```bash
. /home/mariarahel/src/esp-idf/export.sh
```

### Build Commands
Compile the binary prototype from the workspace root:
```bash
idf.py -C tsfi2-deepseek/firmware build
```

### Flash Commands
Deploy the compiled firmware binary sequentially to target hardware:
```bash
# Flash Node A
idf.py -C tsfi2-deepseek/firmware -p /dev/ttyACM0 flash

# Flash Node B
idf.py -C tsfi2-deepseek/firmware -p /dev/ttyACM1 flash
```

## 3. Automated Diagnostic Test Runner

You can execute the entire compilation, flashing, and boot logs check automatically using the custom suite runner:
```bash
python3 scripts/run_firmware_test_suite.py
```

To run a bidirectional mock-free transmission sequence across both active ACM ports:
```bash
python3 scripts/test_radio_chat.py
```

## 4. Troubleshooting
* **Serial Port Permissions**: If connection fails, check permissions using `ls -l /dev/ttyACM*`.
* **CMake Mismatches**: The component `lmic-esp-idf` is globally excluded in `CMakeLists.txt` to prevent transitive dependency errors. Do not remove this exclusion.
