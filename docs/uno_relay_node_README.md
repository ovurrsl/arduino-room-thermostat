---
**Navigation:**
[README](../README.md) | [Architecture](architecture.md) | [BLE Protocol](protocol.md) | [Wiring](wiring.md) | [GIGA UI](giga_ui_sensor_README.md) | [UNO Node](uno_relay_node_README.md) | [Gemini Context](gemini_context.md) | [Contributing](../CONTRIBUTING.md)
---

# UNO R4 Firmware (Relay Node)

This node listens for BLE commands and physically toggles the relays.

## Responsibilities
- Operates as a BLE Peripheral.
- Advertises `ThermostatService`.
- Exposes Characteristics for `temp_C`, `setpoint_C`, and `relay_state`.
- Runs a safety hysteresis loop. If the active temperature falls below the target (minus hysteresis), the heating relay triggers.

## Setup
Compile and flash via Arduino IDE. Ensure the board is set to `Arduino UNO R4 WiFi`.
