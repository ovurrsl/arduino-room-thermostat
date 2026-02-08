# Architecture

## Overview
- **GIGA (Central)**: reads BME680, renders UI, computes control output, sends relay command over BLE.
- **UNO R4 (Peripheral)**: receives relay command, drives relay output, reports relay state.

## Data Flow
1. BME680 → GIGA reads telemetry (T/RH/Pressure/IAQ)
2. GIGA UI updates setpoint and shows current status
3. GIGA computes heating state using hysteresis
4. GIGA → UNO BLE write `relay_cmd` (ON/OFF)
5. UNO → GIGA BLE notify `relay_state`

## Fail-safe
- UNO forces relay OFF when BLE disconnects or on invalid command payload.
