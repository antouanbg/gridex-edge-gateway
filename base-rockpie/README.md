# ROCK Pi E base module

Linux/ARM64 executable for the GrideX base module.

## Interfaces

- GbE / OT: static network to STE-261L, Modbus TCP port 3200, unit ID 1.
- 100 MbE / WAN: OpenRemote, IBEX, VPN, NTP and updates.
- UART plus isolated transceiver: RS485 MBUS backbone to T-CAN485 nodes.

The OT interface must not have a default gateway. Linux IP forwarding remains disabled and the firewall blocks forwarding between OT and WAN.

`GRIDEX_MAX_CHARGE_KW` and `GRIDEX_MAX_DISCHARGE_KW` are optional operator caps. They can only reduce the live limits read from BMS registers 127/128; leaving them empty uses the BMS limits unchanged.

## OpenRemote northbound endpoint

The service exposes the normalized Modbus TCP map on port `1502`, unit ID `1`. OpenRemote must write requested power and enable first, then increment `command.sequence` last. It refreshes `command.ems_heartbeat` at least every 10 seconds; if neither heartbeat nor sequence changes within the 15-second Edge timeout, the applied command becomes `0 kW`.

Bind `GRIDEX_NORTHBOUND_BIND` to the management/WAN interface in production and restrict port `1502` in the firewall to the OpenRemote server. Never expose vendor port `3200` outside the isolated OT interface.

## Build

~~~bash
cmake -S base-rockpie -B build-rockpie
cmake --build build-rockpie
~~~

For the first bench run leave all `GRIDEX_APPROVE_*` values at zero. This permits telemetry reads but locks heartbeat and power writes until direct addressing, sign and scale are verified against the real cabinet. The manufacturer confirmation is recorded, but the on-site readback and limited-power test are still mandatory.

## Safe-state

Heartbeat registers 5301/5302 are refreshed locally only after commissioning approval. If Linux, the service or the OT link fails, the cabinet's own heartbeat timeout returns PCS power to zero. The strategy/cloud path is not part of this safety chain.
