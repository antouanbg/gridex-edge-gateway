# Handoff — GrideX Edge gateway

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## Purpose

This document records work that is planned but not yet commissioned or fully
implemented. It is not a runtime configuration file and contains no secrets,
addresses or customer inventory.

## Pending work

1. **Review and merge the ROCK Pi health publisher**
   - Source: draft PR #2 (`feat/edge-health-publisher`).
   - The publisher sends a bounded health envelope through the Site Router VPN;
     ROCK Pi remains outside WireGuard.

2. **Build the commissioned ROCK Pi image**
   - Install CMake and `libmosquitto` in the ARM64 build environment.
   - Build the complete target, run CTest and validate that the optional MQTT
     publisher reconnects without affecting PCS heartbeat or safe state.

3. **Commission the private MQTT path**
   - Set site/gateway identifiers and the private broker URL through deployment
     configuration, not through Git.
   - Use the Site Router’s VPN-only route. Do not use a public listener,
    WireGuard on ROCK Pi, or a route from backend to the OT/BESS network.
   - Extend the backend MQTT ingestion service to consume the new node
     telemetry topics and publish only authorized node-command topics described
     in `docs/ROCKPI_ESP32_MQTT_BRIDGE.md`. Record audit/result state in
     PostgreSQL; do not give browser clients broker credentials.

4. **Commission one concrete ESP32-EVB driver at a time**
   - Each node must use one compiled driver for one device type, brand, model
     and protocol revision.
   - Confirm CAN/RS-485 settings, sign, scale, byte/word order and allowed
     writes from manufacturer documentation before enabling commands.
   - The Deye SUN-100K-G03 driver (ID `1001`) uses the confirmed official
     string-inverter contract: Modbus RTU `9600 8N1`, default unit ID `1`,
     function `0x06`, and register `77` (`0…1000`, `0.1%`) for active-power
     regulation. It reads back register `77` with `0x03`; register `76` can
     be enabled only for firmware that requires it.
   - Writes are **disabled by default**. Bench-validate the installed unit ID,
     serial settings, model/type response, rated-power scale, register-77
     readback and actual AC output (registers 86/87) before an administrator
     enables `writes_enabled`. Start with a bounded curtailment test; do not
     test on a live production schedule without explicit authorization.
   - Registers `244/245` are hybrid-only and are excluded from this string
     inverter driver. Do not derive other Deye writes from another family or
     from the Suntech TCP map. The approved Suntech SunStorage Pro 261 path
     remains direct ROCK Pi E-to-cabinet Modbus TCP on the isolated OT
     interface.

5. **Bench-test the ROCK Pi ↔ ESP32 MBUS v4 contract**
   - Validate Modbus TCP port 1502/unit 1, source-IP admission, identity and
     telemetry reads, command sequence, 1–30 second TTL, rejection codes and
     zero-power fallback.
   - Test node loss, ROCK Pi restart and PCS communication loss without live
     power writes unless explicitly authorized.

6. **Complete deployment hardening**
   - Add target OS package/service provisioning, firewall rules and health-log
     rotation as deployment artifacts.
   - Bind management services to the approved interface only; never expose
     vendor Modbus port 3200 outside the isolated OT network.

7. **Optional: add the OLIMEX MOD-RS485-ISO ESP32 adapter**
   - Direct UART/RS-485 remains the default node path. Revisit this only where
     a specific device or installation requires galvanic isolation.
   - The module is a PIC-firmware UEXT bridge, not a direct UART transceiver;
     if selected, implement its firmware-version-specific RS-232 or I²C host
     protocol behind `IDownstreamModbusClient`.
   - Verify the module PIC/firmware revision, host protocol/address and
     half/full-duplex jumper configuration on the actual unit before build.

8. **Repair the ROCK Pi CTest suite**
   - The complete CMake build now succeeds locally, but CTest has two existing
     harness issues: the northbound test depends on a local TCP bind that can
     fail in restricted environments, and `gridex_edge_tests` is registered
     without its executable being generated.
   - Make the test port injectable/ephemeral and align test registration with
     the target build. Completion requires `ctest --output-on-failure` to pass
     after a clean CMake build.

## Completion evidence

- PR #2 passes target ARM64 CMake/CTest and review.
- A bench setup shows correct telemetry, control TTL expiry and safe fallback.
- The backend receives Edge health through the private MQTT route only.

## Next action

Provision an ARM64 ROCK Pi build environment with CMake and `libmosquitto`,
then perform the MBUS v4 bench test with a single ESP32-EVB node.
