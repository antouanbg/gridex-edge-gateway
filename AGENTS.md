# GrideX Edge Gateway — Engineering Rules

## Architecture and safety

- The Site Router, not ROCK Pi or ESP32, terminates the WireGuard tunnel.
- ESP32 nodes are on the OT network. ROCK Pi E polls them through Modbus TCP
  and is the sole bridge to private MQTT through the Site Router VPN.
- Do not add WireGuard, public MQTT credentials or direct cloud commands to an
  ESP32 node.
- Vendor maps, signs, scaling and serial/CAN details belong to a device driver.
  Commands must be validated, clamped, time-limited, logged and fail safe.

## OLIMEX ESP32-EVB — verified local bench knowledge

The supported node board family is OLIMEX ESP32-EVB / ESP32-EVB-EA-IND.

- On the verified bench board, built-in Relay 1 is GPIO32 and Relay 2 is GPIO33.
- The tested relay profile is **active-high**: `HIGH` energises a relay and
  `LOW` releases it. Keep this configurable for a different board revision and
  validate it during commissioning.
- The relays operated successfully while the board was powered from USB 5 V;
  no separate relay supply was used for the bench test.
- PlatformIO environment: `board = esp32-evb`, Arduino framework.
- The USB serial console uses 115200 bps. Keep one serial connection open while
  issuing a bench command: opening and closing the port for each command can
  reset the ESP32 and lose the command.
- Upload at `115200` bps on this setup. The higher 921600 bps upload rate was
  observed to corrupt the transfer after the ESP32 bootloader connected.

## Relay testing

- Relay testing is permitted only when the user has explicitly confirmed that
  the relevant relay is free of live or safety-critical loads.
- Start every test with both outputs OFF. Use a short bounded pulse and an
  independent firmware timeout; always send an explicit OFF afterward.
- The temporary test firmware is under
  `node-esp32-evb/examples/relay-test/`. Its serial commands are `r1 on`,
  `r1 off`, `r2 on`, `r2 off`, `all off` and `status`.
- The diagnostic firmware currently contains a startup self-test that pulses
  both relays. **It must never be used when real loads are connected**, and it
  must be replaced by the production node firmware before commissioning.
- Production commands must not use this USB test interface. They must arrive
  only through the approved ROCK Pi E / OT Modbus TCP command path and retain
  the configured safety envelope, TTL and safe-zero behavior.

## Repository hygiene

- Never commit device-specific credentials, VPN keys, customer addresses,
  production IP ranges, inventory identifiers or USB device paths.
- Maintain English technical documentation with a matching Bulgarian section
  whenever user-facing or operational documentation is changed.
- Before changing firmware, inspect `git status`; preserve unrelated work.
