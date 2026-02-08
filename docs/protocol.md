# BLE Protocol

## Roles
- **GIGA (Central)**: decides ON/OFF and writes commands
- **UNO R4 (Peripheral)**: controls relay and notifies current state

## Service
- Name: `ThermostatService`
- UUID: `REPLACE-WITH-YOUR-UUID`

## Characteristics

### Relay Command
- Name: `relay_cmd`
- Properties: Write
- UUID: `REPLACE-WITH-YOUR-UUID`
- Payload: 1 byte
  - `0x00` = OFF
  - `0x01` = ON

### Relay State
- Name: `relay_state`
- Properties: Notify
- UUID: `REPLACE-WITH-YOUR-UUID`
- Payload: 1 byte
  - `0x00` = OFF
  - `0x01` = ON

## Fail-safe Rules
- On BLE disconnect: UNO forces relay OFF.
- UNO ignores invalid payloads (anything other than 0 or 1).
