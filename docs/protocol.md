---
**Navigation:**
[README](../README.md) | [Architecture](architecture.md) | [BLE Protocol](protocol.md) | [Wiring](wiring.md) | [GIGA UI](giga_ui_sensor_README.md) | [UNO Node](uno_relay_node_README.md) | [Gemini Context](gemini_context.md) | [Contributing](../CONTRIBUTING.md)
---

# BLE Protocol

## Roles
- **GIGA R1 (Central)**: Reads sensor data, sends actual temperature and target setpoint. Subscribes to Relay State.
- **UNO R4 (Peripheral)**: Computes logic, controls relays, notifies the Central of state changes.

## Service
- **Name**: `ThermostatService`
- **UUID**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`

## Characteristics

### Temperature (C)
- **Name**: `temp_C`
- **Properties**: Write Without Response
- **UUID**: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- **Payload**: 4 bytes (Float32)

### Setpoint (C)
- **Name**: `setpoint_C`
- **Properties**: Write Without Response
- **UUID**: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- **Payload**: 4 bytes (Float32)

### Relay State
- **Name**: `relay_state`
- **Properties**: Notify / Read
- **UUID**: `6E400004-B5A3-F393-E0A9-E50E24DCCA9E`
- **Payload**: 1 byte
  - `0x00` = OFF
  - `0x01` = ON

### System Mode
- **Name**: `system_mode`
- **Properties**: Write Without Response
- **UUID**: `6E400005-B5A3-F393-E0A9-E50E24DCCA9E`
- **Payload**: 1 byte
  - `0x00` = Heating
  - `0x01` = Cooling
  - `0x02` = Fan
  - `0x03` = Off

### Hysteresis (C)
- **Name**: `hysteresis`
- **Properties**: Write Without Response
- **UUID**: `6E400006-B5A3-F393-E0A9-E50E24DCCA9E`
- **Payload**: 4 bytes (Float32)

## Reliability
- Float values are transmitted as raw 4-byte arrays. Both platforms are 32-bit ARM Cortex processors, ensuring identical float representations (IEEE 754).
