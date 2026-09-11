# Handoff — GrideX Edge gateway

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

4. **Commission one concrete ESP32-EVB driver at a time**
   - Each node must use one compiled driver for one device type, brand, model
     and protocol revision.
   - Confirm CAN/RS-485 settings, sign, scale, byte/word order and allowed
     writes from manufacturer documentation before enabling commands.

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

7. **Add the OLIMEX MOD-RS485-ISO ESP32 adapter**
   - This selected module is a PIC-firmware UEXT bridge, not a direct UART
     transceiver. Implement its firmware-version-specific RS-232 or I²C host
     protocol behind `IDownstreamModbusClient`.
   - Verify the module PIC/firmware revision, host protocol/address and
     half/full-duplex jumper configuration on the actual unit before build.
   - Bench-test read-only Modbus RTU, then the permitted write path, while
     keeping all vendor limits and failsafe behaviour intact.

## Completion evidence

- PR #2 passes target ARM64 CMake/CTest and review.
- A bench setup shows correct telemetry, control TTL expiry and safe fallback.
- The backend receives Edge health through the private MQTT route only.

## Next action

Provision an ARM64 ROCK Pi build environment with CMake and `libmosquitto`,
then perform the MBUS v4 bench test with a single ESP32-EVB node.
