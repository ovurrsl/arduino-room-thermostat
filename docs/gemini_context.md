---
**Navigation:**
[README](../README.md) | [Architecture](architecture.md) | [BLE Protocol](protocol.md) | [Wiring](wiring.md) | [GIGA UI](giga_ui_sensor_README.md) | [UNO Node](uno_relay_node_README.md) | [Gemini Context](gemini_context.md) | [Contributing](../CONTRIBUTING.md)
---

# Gemini AI Context

## Project Overview
This file serves as the main context and rulebook for the AI (Gemini) working on the Arduino Room Thermostat project. The goal is to provide a unified source of truth for the system state, conventions, and next steps.

## System State
- **GIGA UI Node:** Successfully reads from the BME680 sensor (via I2C address 0x77). An LVGL-based UI runs on the Arduino GIGA Display Shield, featuring an arc slider for target temperature, a dashboard for Humidity/Pressure/Gas Resistance, and operating mode buttons.
- **BLE Communication:** GIGA is the Central, UNO R4 is the Peripheral. GIGA sends the actual and target temperatures. UNO R4 calculates the heating requirement using a hysteresis model and sends back the relay state.
- **I2C Scanner:** An I2C scanner utility was added under `firmware/i2c_scanner` for debugging the BME680 or future I2C displays.

## Guidelines for AI
1. **Always use LVGL for GIGA Display updates.** The project uses the `Arduino_H7_Video`, `Arduino_GigaDisplayTouch`, and `lvgl` libraries.
2. **Follow BLE standard practices.** `Write Without Response` is used for frequent telemetry, and `Notify` is used for state changes.
3. **Maintain robust fail-safes.** If BLE disconnects or sensors fail, safely disable heating systems (Relay OFF).
4. **Update Documentation:** Whenever new features are introduced, update the respective Markdown files to reflect reality, and ensure navigation links remain intact.
