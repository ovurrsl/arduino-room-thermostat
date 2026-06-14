---
**Navigation:**
[README](../README.md) | [Architecture](architecture.md) | [BLE Protocol](protocol.md) | [Wiring](wiring.md) | [GIGA UI](giga_ui_sensor_README.md) | [UNO Node](uno_relay_node_README.md) | [Gemini Context](gemini_context.md) | [Contributing](../CONTRIBUTING.md)
---

# Wiring & Hardware

## 1. GIGA R1 WiFi + Display
- **Display**: Directly mounted on the GIGA R1 headers (Giga Display Shield).
- **BME680 Sensor (I2C)**:
  - `VCC` → `3.3V`
  - `GND` → `GND`
  - `SDA` → `Pin 20 (SDA)`
  - `SCL` → `Pin 21 (SCL)`
  - Note: The BME680 default I2C address is usually `0x77`. Check using `firmware/i2c_scanner` if issues arise.

## 2. UNO R4 WiFi + Relays Shield
- **Shield**: Directly mounted on the UNO R4 headers.
- **Relay Map**:
  - `R1` = Pin `D4` (Used for Heating Control)
  - `R2` = Pin `D7`
  - `R3` = Pin `D8`
  - `R4` = Pin `D12`
