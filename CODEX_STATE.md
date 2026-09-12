# CODEX_STATE.md

## Current task

Open a Pull Request for the completed Edge checkpoint and maintain the
mandatory review workflow in this repository.

Създай Pull Request за завършения Edge checkpoint и поддържай задължителния
review процес в това repository.

## Completed

- ROCK Pi E booted the approved pilot image; native ARM64 build and both CTest
  suites passed on the physical board.
- The ROCK Pi service and locked commissioning configuration were installed.
  The service is disabled and inactive; every write-approval gate remains `0`.
- OLIMEX ESP32-EVB RS485/Ethernet firmware was built and flashed over USB.
- ROCK Pi verified read-only Modbus TCP identity and telemetry-range reads from
  the ESP32 through the temporary management-LAN bench connection.
- The node stayed unconfigured, returned expected zero telemetry values and
  did not send a downstream RS485 request.
- The source code reads Suntech cumulative-energy registers 122–125 atomically
  in one Modbus `0x04` request and publishes PCS operating state northbound.
- Added the mandatory rule: every completed change is pushed and opened as a
  Pull Request before it is reported as ready; automatic merging is forbidden.

## Remaining

- Obtain the exact Deye test-inverter model/revision, RS485 A/B/GND wiring,
  unit ID and complete manufacturer-approved read-register map.
- Implement and bench-test only a read-only Deye driver from that verified map.
- Commission the isolated OT Ethernet network, Site Router firewall rules and
  approved site configuration before enabling any service.
- Complete read-only Suntech verification, telemetry soak, backend ingestion
  and OpenRemote mapping. No BESS/PCS write path is approved.

## Modified files

- `AGENTS.md`
- `HANDOFF.md`
- `docs/ROCKPI_E_PILOT_STATUS.md`
- `CODEX_STATE.md`
- `AGENTS.md`

## Tests

- Native ROCK Pi E ARM64 build: passed.
- `gridex_rockpie_tests` and `gridex_edge_tests` on ROCK Pi E: passed.
- ESP32-EVB CAN and RS485 PlatformIO builds: passed.
- ROCK Pi to ESP32 canonical Modbus TCP identity and telemetry-range reads:
  passed, read-only.

## Known issues

- The OT Ethernet has no physical carrier and is not commissioned.
- The ESP32 driver is intentionally unconfigured; its zero telemetry is not
  inverter data.
- The Deye map supplied so far does not yet provide the complete read map for
  the exact physically connected model.

## Next action

Review the Pull Request for this checkpoint. Then read `AGENTS.md`,
`HANDOFF.md` and this file; inspect `git status`; obtain and validate the exact
Deye model and read-register map before touching the RS485 driver. Keep all
services disabled and all `GRIDEX_APPROVE_*` flags at `0`.

## Last updated

2026-09-12 — documentation checkpoint after the read-only ROCK Pi ↔ ESP32
bench verification.
