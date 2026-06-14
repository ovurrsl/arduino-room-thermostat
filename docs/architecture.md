---
**Navigation:**
[README](../README.md) | [Architecture](architecture.md) | [BLE Protocol](protocol.md) | [Wiring](wiring.md) | [GIGA UI](giga_ui_sensor_README.md) | [UNO Node](uno_relay_node_README.md) | [Gemini Context](gemini_context.md) | [Contributing](../CONTRIBUTING.md)
---

# Architecture

## Overview
- **GIGA R1 (Central)**: Handles the LVGL user interface and I2C BME680 sensor readings. It gathers telemetry and user inputs (setpoints, modes) and streams them to the UNO over BLE.
- **UNO R4 (Peripheral)**: Acts as the actuator. Receives target and current temperatures, computes the heating/cooling state using hysteresis logic, drives the physical relay, and sends its real-time state back to the GIGA.

## Data Flow
1. **Sensing**: BME680 → GIGA reads T/RH/Pressure/Gas via I2C (0x77).
2. **UI & State**: GIGA updates LVGL objects to reflect the current room state. User interacts with the UI to change the target temperature.
3. **Transmission**: GIGA (Central) → UNO (Peripheral) writes `temp_C` and `setpoint_C` over BLE Characteristics.
4. **Execution**: UNO receives values, runs hysteresis checks (`T <= SP - H` = ON, `T >= SP + H` = OFF), and switches Relay 1.
5. **Feedback**: UNO → GIGA notifies `relay_state`. GIGA UI reflects the active heating/cooling state.

## Fail-safe Mechanisms
- **Sensor Failure**: GIGA detects I2C read failures and shows errors on the UI.
- **Connection Loss**: If BLE disconnects, the UNO node defaults the relay to OFF immediately.
