# ROCK Pi E base module

Linux/ARM64 executable for the GrideX base module.

## Interfaces

- GbE / OT: static network to STE-261L, Modbus TCP port 3200, unit ID 1.
- 100 MbE / WAN: OpenRemote, IBEX, VPN, NTP and updates.
- UART plus isolated transceiver: RS485 MBUS backbone to T-CAN485 nodes.

`gridex-rockpie-service` включва постоянен polling worker. Той обхожда адресите
от `GRIDEX_MBUS_NODE_ADDRESSES` през интервала `GRIDEX_MBUS_POLL_MS`, чете
identity и telemetry блоковете и ги публикува в нормализираната карта. Един
неотговарящ нод не прекъсва обхождането на останалите.

The OT interface must not have a default gateway. Linux IP forwarding remains disabled and the firewall blocks forwarding between OT and WAN.

`GRIDEX_MAX_CHARGE_KW` and `GRIDEX_MAX_DISCHARGE_KW` are optional operator caps. They can only reduce the live limits read from BMS registers 127/128; leaving them empty uses the BMS limits unchanged.

## OpenRemote northbound endpoint

The service exposes the normalized Modbus TCP map on port `1502`, unit ID `1`. OpenRemote must write requested power and enable first, then increment `command.sequence` last. It refreshes `command.ems_heartbeat` at least every 10 seconds; if neither heartbeat nor sequence changes within the 15-second Edge timeout, the applied command becomes `0 kW`.

Holding registers 4-10 form a separate operator-only transaction. OpenRemote
writes the action mask and requested values, apply key `0xA55A`, then increments
operator sequence last. Result code and processed sequence are returned in
input registers 26-27. This path is not writable by the automatic strategy.

Bind `GRIDEX_NORTHBOUND_BIND` to the management/WAN interface in production and restrict port `1502` in the firewall to the OpenRemote server. Never expose vendor port `3200` outside the isolated OT interface.

Нодовете са в input-register слотове с начало `0x0100`, 16 регистъра на нод и
до 32 нода. Слотът съдържа online, MBUS address, node type/state, driver ID,
quality, heartbeat, power, energy, device state, alarms, age и дали директната
MQTTS връзка на нода е активна.

## Build

~~~bash
cmake -S base-rockpie -B build-rockpie
cmake --build build-rockpie
~~~

For the first bench run leave all `GRIDEX_APPROVE_*` values at zero. This permits telemetry reads but locks heartbeat and power writes until direct addressing, sign and scale are verified against the real cabinet. The manufacturer confirmation is recorded, but the on-site readback and limited-power test are still mandatory.

`GRIDEX_APPROVE_INT32_WORD_ORDER` affects only validity of accumulated-energy
telemetry from registers 122-125. The vendor table declares Int32 but does not
specify the two-word order, so this flag remains zero until a meter comparison
confirms `GRIDEX_INT32_HIGH_WORD_FIRST` for the real cabinet.

## Safe-state

Heartbeat registers 5301/5302 are refreshed locally only after commissioning approval. If Linux, the service or the OT link fails, the cabinet's own heartbeat timeout returns PCS power to zero. The strategy/cloud path is not part of this safety chain.
