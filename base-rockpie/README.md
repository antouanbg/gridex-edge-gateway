# ROCK Pi E base module

## English

Linux/ARM64 executable for the GrideX base module.

## Interfaces

- GbE / OT: static network to STE-261L, Modbus TCP port 3200, unit ID 1.
- 100 MbE / WAN: OpenRemote, IBEX, VPN, NTP and updates.
- GbE / OT switch: Modbus TCP to each OLIMEX ESP32-EVB node.

`gridex-rockpie-service` continuously polls the comma-separated
`GRIDEX_NODE_ENDPOINTS` over OT Ethernet at `GRIDEX_NODE_POLL_MS`. It reads
the canonical identity and telemetry blocks and publishes them into the
northbound map. One non-responsive node does not interrupt the others.

The OT interface must not have a default gateway. Linux IP forwarding remains disabled and the firewall blocks forwarding between OT and WAN.

## Edge health over VPN-only MQTT

When `GRIDEX_MQTT_BROKER_URL`, `GRIDEX_SITE_ID` and `GRIDEX_GATEWAY_ID` are set,
the service publishes a non-secret health envelope every ten seconds to
`gridex/v1/sites/<site-id>/edge/<gateway-id>/health`. The broker is reached
through the **Site Router's WireGuard route**. ROCK Pi does not run WireGuard,
does not publish to a public MQTT listener, and never forwards the OT/BESS
network. The health message includes only service state, PCS heartbeat state,
safe-mode/control readiness and node availability.

The binary enables MQTT publishing when `libmosquitto` is available at build
time. Commissioned production images must install that package and configure
certificate/credential secrets outside this repository.

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

Holding registers 11–16 route a device command to one node slot. OpenRemote
writes target slot, requested power, enable, TTL (1–30 seconds) and apply key,
then changes the node-command sequence. ROCK Pi E forwards the complete command
over OT Modbus TCP. The node accepts only the configured ROCK Pi E source and
forces 0 kW when the TTL expires.

## Build

~~~bash
cmake -S base-rockpie -B build-rockpie
cmake --build build-rockpie
~~~

For the first bench run leave all `GRIDEX_APPROVE_*` values at zero. This permits telemetry reads but locks heartbeat and power writes until direct addressing, sign and scale are verified against the real cabinet. The manufacturer confirmation is recorded, but the on-site readback and limited-power test are still mandatory.

`GRIDEX_APPROVE_INT32_WORD_ORDER` unlocks accumulated-energy telemetry from
registers 122–125 after the on-site check. The manufacturer confirmed ABCD /
high-order word first and signed Int32 divided by 10.

## Safe-state

Heartbeat registers 5301/5302 are refreshed locally only after commissioning approval. If Linux, the service or the OT link fails, the cabinet's own heartbeat timeout returns PCS power to zero. The strategy/cloud path is not part of this safety chain.

---

## Български

Linux/ARM64 изпълним модул за базовото GrideX устройство.

### Интерфейси

- GbE / OT: статична мрежа към STE-261L, Modbus TCP порт 3200, unit ID 1.
- 100 MbE / WAN: OpenRemote, IBEX, VPN, NTP и обновявания.
- GbE / OT switch: Modbus TCP към всеки OLIMEX ESP32-EVB нод.

`gridex-rockpie-service` обхожда постоянно endpoint-ите от
`GRIDEX_NODE_ENDPOINTS` по OT Ethernet през `GRIDEX_NODE_POLL_MS`. Един
неотговарящ нод не прекъсва останалите.

OT интерфейсът няма default gateway. Linux IP forwarding е изключен, а firewall-ът блокира препращането между OT и WAN.

### Edge health през MQTT само по VPN

При зададени `GRIDEX_MQTT_BROKER_URL`, `GRIDEX_SITE_ID` и
`GRIDEX_GATEWAY_ID` услугата публикува несекретен health пакет на всеки десет
секунди към `gridex/v1/sites/<site-id>/edge/<gateway-id>/health`. Брокерът се
достига през WireGuard маршрута на **Site Router**. ROCK Pi не изпълнява
WireGuard, не публикува към публичен MQTT listener и не препраща OT/BESS
мрежата. Пакетът съдържа само състояние на услугата, PCS heartbeat, safe-mode /
control readiness и наличност на нодовете.

`GRIDEX_MAX_CHARGE_KW` и `GRIDEX_MAX_DISCHARGE_KW` са опционални операторски лимити. Те могат само да намалят текущите BMS лимити от регистри 127/128; празни стойности използват BMS лимитите без промяна.

### Northbound endpoint към OpenRemote

Услугата предоставя нормализираната Modbus TCP карта на порт `1502`, unit ID `1`. OpenRemote първо записва желаната мощност и enable, след което увеличава `command.sequence`. `command.ems_heartbeat` се обновява поне на 10 секунди; ако heartbeat или sequence не се променят в рамките на 15 секунди, приложената команда става `0 kW`.

Holding регистри 4–10 образуват отделна operator-only транзакция. OpenRemote записва action mask и стойностите, apply key `0xA55A`, след което увеличава operator sequence. Result code и обработената sequence стойност се връщат в input регистри 26–27. Автоматичната стратегия няма право да записва по този път.

В production `GRIDEX_NORTHBOUND_BIND` се свързва с management/WAN интерфейса, а firewall-ът допуска порт `1502` само от OpenRemote сървъра. Vendor порт `3200` никога не се публикува извън изолирания OT интерфейс.

Нодовете са в input-register слотове от `0x0100`, по 16 регистъра на нод и до 32 нода. Слотът съдържа online състояние, MBUS адрес, node type/state, driver ID, quality, heartbeat, power, energy, device state, alarms, възраст на данните и състояние на директната MQTTS връзка.

Holding регистри 11–16 маршрутизират команда към конкретен node slot.
ROCK Pi E я препраща по OT Modbus TCP. Нодът допуска само конфигурирания
ROCK Pi E и при изтичане на TTL връща командата към 0 kW.

### Компилиране

~~~bash
cmake -S base-rockpie -B build-rockpie
cmake --build build-rockpie
~~~

При първия стендов тест всички `GRIDEX_APPROVE_*` стойности остават нула. Това позволява telemetry reads, но заключва heartbeat и power writes, докато адресиране, знак и мащаб не бъдат потвърдени върху реалния шкаф.

`GRIDEX_APPROVE_INT32_WORD_ORDER` отключва accumulated-energy телеметрията от
регистри 122–125 след проверка на място. Производителят потвърди ABCD /
high-order word first и signed Int32 ÷10.

### Безопасно състояние

Heartbeat регистрите 5301/5302 се обновяват локално само след commissioning approval. При отказ на Linux, услугата или OT връзката, собственият heartbeat timeout на шкафа връща PCS мощността към нула. Стратегията и облакът не са част от тази защитна верига.
