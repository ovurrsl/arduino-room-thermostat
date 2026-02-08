# Arduino Room Thermostat (GIGA + UNO R4)

A room thermostat built with two boards:
- **Arduino GIGA R1 + GIGA Display**: UI, sensor reading (BME680), control logic
- **Arduino UNO R4 WiFi + 4 Relays Shield**: relay driver node (heating ON/OFF)

Boards communicate over **Bluetooth Low Energy (BLE)**.

## Features (MVP)
- Live temperature/humidity/pressure/air quality (BME680)
- Setpoint control from the UI
- Hysteresis-based heating control
- BLE command + state feedback
- Fail-safe behavior: relay defaults to OFF on disconnect

## Hardware
- Arduino GIGA R1 WiFi + GIGA Display
- Adafruit BME680 (I2C)
- Arduino UNO R4 WiFi + 4 Relays Shield

## Project Structure
- `firmware/giga_ui_sensor` — UI + sensor + control decision
- `firmware/uno_relay_node` — relay control + status notify
- `docs/protocol.md` — BLE service/characteristics + payloads
- `docs/wiring.md` — wiring notes

## Control Logic (default)
- If `T <= SP - H` → HEATING ON
- If `T >= SP + H` → HEATING OFF

## Safety Note
This project may be used to control mains-powered heating equipment. Use proper isolation, fusing, enclosure, and an appropriate SSR/contactor design. Always comply with local electrical safety regulations.

## License
MIT
