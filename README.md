---
**Navigation:**
[README](README.md) | [Architecture](docs/architecture.md) | [BLE Protocol](docs/protocol.md) | [Wiring](docs/wiring.md) | [GIGA UI](docs/giga_ui_sensor_README.md) | [UNO Node](docs/uno_relay_node_README.md) | [Gemini Context](docs/gemini_context.md) | [Contributing](CONTRIBUTING.md)
---

# Arduino Room Thermostat (GIGA + UNO R4)

A smart room thermostat built with two Arduino boards:
- **Arduino GIGA R1 WiFi + GIGA Display**: Features a modern LVGL graphical user interface, reads room metrics via a BME680 sensor, and acts as the central control logic.
- **Arduino UNO R4 WiFi + 4 Relays Shield**: Acts as a remote relay driver node that controls heating (ON/OFF) based on instructions received.

The boards communicate securely over **Bluetooth Low Energy (BLE)**.

## Features (Current)
- **Modern UI**: Full touch interface using LVGL (Arc sliders, dashboards, mode selectors).
- **Environment Dashboard**: Live temperature, humidity, pressure, and air quality (Gas Resistance) via BME680.
- **Air Quality Warnings**: UI & serial alerts when IAQ drops below a threshold.
- **Setpoint Control**: Drag to set the target temperature.
- **BLE Telemetry**: GIGA central syncs targets and readings with the UNO peripheral.
- **Fail-safe Behavior**: Relays default to OFF if BLE disconnects.

## Planned Features (Roadmap)
- Wi-Fi and MQTT integration for Smart Home systems (e.g., Home Assistant).
- Historical charts and trends on the GIGA display.

## Project Structure
- `firmware/giga_ui_sensor` — GIGA Display UI (LVGL) + BME680 Sensor + BLE Central.
- `firmware/uno_relay_node` — Control logic, hysteresis + Relay Driver (BLE Peripheral).
- `firmware/i2c_scanner` — Utility to debug I2C devices.
- `docs/` — System architecture, BLE protocol, wiring diagrams, and AI contexts.

## License
MIT
