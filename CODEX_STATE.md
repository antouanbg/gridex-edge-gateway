# Current task

Implement the ROCK Pi MQTT bridge for ESP32 node telemetry and commands while
keeping ESP32 nodes on internal OT Modbus TCP only.

## Completed

- Added the `esp32-evb-deye-sun100k-g03-rs485` PlatformIO target.
- Added a direct UART/RS-485 Modbus RTU read client and the Deye telemetry
  driver (ID 1001).
- Added the bilingual Deye profile and driver-catalog record.
- Compiled the ESP target and ran the host MBUS test successfully.
- Added ROCK Pi node telemetry publishing and bounded MQTT command intake.
- Disabled direct ESP MQTT by default and documented the MQTT topic contract.

## Remaining

- Bench-validate the installed Deye unit's serial settings, unit ID, model
  response, scale and alarms.
- Implement backend MQTT ingestion, authorization and audit for the new node
  telemetry and command topics.
- Obtain and validate the exact Deye active-power-limit write contract before
  enabling any Deye command.
- Complete physical commissioning; no firmware has been flashed to hardware.

## Modified files

- `node-esp32-evb/` Deye driver, RS-485 client and PlatformIO target.
- `docs/DEYE_SUN100K_G03_RS485_PROTOCOL.md`
- `docs/ROCKPI_ESP32_MQTT_BRIDGE.md`
- `docs/DRIVER_CATALOG.md`
- `HANDOFF.md`

## Tests

- `pio run -d node-esp32-evb -e esp32-evb-deye-sun100k-g03-rs485` — passed.
- Host MBUS CMake/CTest suite — passed (1/1).
- ROCK Pi CMake build with MQTT bridge — passed.
- ROCK Pi CTest — partial: `gridex_rockpie_tests` passed; the pre-existing
  `gridex_edge_tests` registration references a missing executable.

## Known issues

- Deye active-power control is intentionally disabled pending manufacturer and
  on-site validation; the current profile is telemetry-only.

## Next action

Implement backend MQTT ingestion for node topics, then flash a lab ESP32-EVB
and validate the complete ESP32 → ROCK Pi → MQTT → backend route.

## Last updated

2026-09-11
