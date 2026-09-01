# ROCK Pi E base module

## English

Linux/ARM64 executable for the GrideX base module.

## Interfaces

- GbE / OT: static network to STE-261L, Modbus TCP port 3200, unit ID 1.
- 100 MbE / WAN: OpenRemote, IBEX, VPN, NTP and updates.
- UART plus isolated transceiver: RS485 MBUS backbone to T-CAN485 nodes.

`gridex-rockpie-service` includes a continuous polling worker. It scans the
addresses in `GRIDEX_MBUS_NODE_ADDRESSES` at the `GRIDEX_MBUS_POLL_MS` interval,
reads the identity and telemetry blocks and publishes them into the normalized
map. One non-responsive node does not interrupt polling of the other nodes.

The OT interface must not have a default gateway. Linux IP forwarding remains disabled and the firewall blocks forwarding between OT and WAN.

`GRIDEX_MAX_CHARGE_KW` and `GRIDEX_MAX_DISCHARGE_KW` are optional operator caps. They can only reduce the live limits read from BMS registers 127/128; leaving them empty uses the BMS limits unchanged.

## OpenRemote northbound endpoint

The service exposes the normalized Modbus TCP map on port `1502`, unit ID `1`. OpenRemote must write requested power and enable first, then increment `command.sequence` last. It refreshes `command.ems_heartbeat` at least every 10 seconds; if neither heartbeat nor sequence changes within the 15-second Edge timeout, the applied command becomes `0 kW`.

Holding registers 4-10 form a separate operator-only transaction. OpenRemote
writes the action mask and requested values, apply key `0xA55A`, then increments
operator sequence last. Result code and processed sequence are returned in
input registers 26-27. This path is not writable by the automatic strategy.

Bind `GRIDEX_NORTHBOUND_BIND` to the management/WAN interface in production and restrict port `1502` in the firewall to the OpenRemote server. Never expose vendor port `3200` outside the isolated OT interface.

Nodes occupy input-register slots starting at `0x0100`, with 16 registers per
node and support for up to 32 nodes. A slot contains online state, MBUS address,
node type/state, driver ID, quality, heartbeat, power, energy, device state,
alarms, data age and direct-MQTTS connection state.

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

---

## Български

Linux/ARM64 изпълним модул за базовото GrideX устройство.

### Интерфейси

- GbE / OT: статична мрежа към STE-261L, Modbus TCP порт 3200, unit ID 1.
- 100 MbE / WAN: OpenRemote, IBEX, VPN, NTP и обновявания.
- UART плюс изолиран трансивър: RS485 MBUS гръбнак към T-CAN485 нодовете.

`gridex-rockpie-service` включва постоянен polling worker. Той обхожда адресите от `GRIDEX_MBUS_NODE_ADDRESSES` през интервала `GRIDEX_MBUS_POLL_MS`, чете identity и telemetry блоковете и ги публикува в нормализираната карта. Един неотговарящ нод не прекъсва обхождането на останалите.

OT интерфейсът няма default gateway. Linux IP forwarding е изключен, а firewall-ът блокира препращането между OT и WAN.

`GRIDEX_MAX_CHARGE_KW` и `GRIDEX_MAX_DISCHARGE_KW` са опционални операторски лимити. Те могат само да намалят текущите BMS лимити от регистри 127/128; празни стойности използват BMS лимитите без промяна.

### Northbound endpoint към OpenRemote

Услугата предоставя нормализираната Modbus TCP карта на порт `1502`, unit ID `1`. OpenRemote първо записва желаната мощност и enable, след което увеличава `command.sequence`. `command.ems_heartbeat` се обновява поне на 10 секунди; ако heartbeat или sequence не се променят в рамките на 15 секунди, приложената команда става `0 kW`.

Holding регистри 4–10 образуват отделна operator-only транзакция. OpenRemote записва action mask и стойностите, apply key `0xA55A`, след което увеличава operator sequence. Result code и обработената sequence стойност се връщат в input регистри 26–27. Автоматичната стратегия няма право да записва по този път.

В production `GRIDEX_NORTHBOUND_BIND` се свързва с management/WAN интерфейса, а firewall-ът допуска порт `1502` само от OpenRemote сървъра. Vendor порт `3200` никога не се публикува извън изолирания OT интерфейс.

Нодовете са в input-register слотове от `0x0100`, по 16 регистъра на нод и до 32 нода. Слотът съдържа online състояние, MBUS адрес, node type/state, driver ID, quality, heartbeat, power, energy, device state, alarms, възраст на данните и състояние на директната MQTTS връзка.

### Компилиране

~~~bash
cmake -S base-rockpie -B build-rockpie
cmake --build build-rockpie
~~~

При първия стендов тест всички `GRIDEX_APPROVE_*` стойности остават нула. Това позволява telemetry reads, но заключва heartbeat и power writes, докато адресиране, знак и мащаб не бъдат потвърдени върху реалния шкаф.

`GRIDEX_APPROVE_INT32_WORD_ORDER` влияе само върху валидността на accumulated-energy телеметрията от регистри 122–125. Производителската таблица посочва Int32, но не и реда на двете думи; флагът остава нула до сравнение с реален електромер.

### Безопасно състояние

Heartbeat регистрите 5301/5302 се обновяват локално само след commissioning approval. При отказ на Linux, услугата или OT връзката, собственият heartbeat timeout на шкафа връща PCS мощността към нула. Стратегията и облакът не са част от тази защитна верига.
