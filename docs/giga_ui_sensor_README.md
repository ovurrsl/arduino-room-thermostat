---
**Navigation:**
[README](../README.md) | [Architecture](architecture.md) | [BLE Protocol](protocol.md) | [Wiring](wiring.md) | [GIGA UI](giga_ui_sensor_README.md) | [UNO Node](uno_relay_node_README.md) | [Gemini Context](gemini_context.md) | [Contributing](../CONTRIBUTING.md)
---

# GIGA Firmware (UI + Sensor + Central)

This firmware module handles the user-facing side of the thermostat.

## Core Libraries
- `Arduino_H7_Video` & `Arduino_GigaDisplayTouch`: Drivers for the GIGA Display.
- `lvgl`: Renders the UI (Arc slider, buttons, labels). Note: `#include <lvgl.h>` must precede video headers.
- `Adafruit_BME680`: Drives the I2C environment sensor.
- `ArduinoBLE`: Handles Central mode operations to push targets to the UNO.

## Sub-modules
- `giga_ui_sensor.ino`: Main setup and loop, handling BLE discovery, BME680 reading, and LVGL task management.
- `lvgl_ui.h`: Contains all LVGL setup code, UI object references, styling, and event callbacks.

## Setup
Compile and flash via Arduino IDE. Ensure the board is set to `Arduino GIGA R1`.
