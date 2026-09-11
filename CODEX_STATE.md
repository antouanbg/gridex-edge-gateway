# Current task

Implement and compile the first Deye SUN-100K-G03 RS-485 ESP32-EVB telemetry
profile while keeping Suntech SunStorage Pro 261 on direct ROCK Pi E Modbus TCP.

## Completed

- Added the `esp32-evb-deye-sun100k-g03-rs485` PlatformIO target.
- Added a direct UART/RS-485 Modbus RTU read client and the Deye telemetry
  driver (ID 1001).
- Added the bilingual Deye profile and driver-catalog record.
- Compiled the ESP target and ran the host MBUS test successfully.

## Remaining

- Bench-validate the installed Deye unit's serial settings, unit ID, model
  response, scale and alarms.
- Obtain and validate the exact Deye active-power-limit write contract before
  enabling any Deye command.
- Complete physical commissioning; no firmware has been flashed to hardware.

## Modified files

- `node-esp32-evb/` Deye driver, RS-485 client and PlatformIO target.
- `docs/DEYE_SUN100K_G03_RS485_PROTOCOL.md`
- `docs/DRIVER_CATALOG.md`
- `HANDOFF.md`

## Tests

- `pio run -d node-esp32-evb -e esp32-evb-deye-sun100k-g03-rs485` — passed.
- Host MBUS CMake/CTest suite — passed (1/1).

## Known issues

- Deye active-power control is intentionally disabled pending manufacturer and
  on-site validation; the current profile is telemetry-only.

## Next action

Review the Edge pull request, then flash a lab ESP32-EVB and bench-validate
telemetry against the actual Deye SUN-100K-G03 before enabling any write map.

## Last updated

2026-09-11
