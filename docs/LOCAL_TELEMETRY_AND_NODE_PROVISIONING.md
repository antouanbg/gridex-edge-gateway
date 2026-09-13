# Local telemetry journal and ESP32 node provisioning / Локален telemetry журнал и ESP32 node provisioning

## English

### Purpose and safety boundary

The ROCK Pi E service maintains a bounded, durable local telemetry journal for
the ESP32 node slots it polls. It records periodic normalized snapshots and a
separate record whenever a slot changes polling state, including offline,
identity and telemetry failures. This is an **observation-only** function. It
does not replay commands, open an inbound listener, or unlock any commissioning
or device-write gate.

The journal is deliberately independent of MQTT. If the Site Router VPN or the
private broker is unavailable, polling and local journal writes continue. A
future authenticated backend recovery worker may export records using the
approved v1 contract in `docs/TELEMETRY_JOURNAL_RECOVERY_V1.md`. That worker,
exporter and acknowledgement mechanism are not part of this change; no backend
credentials, URLs or customer data are stored in the journal.

### ROCK Pi local journal

The protected deployment environment accepts these generic settings:

```ini
GRIDEX_TELEMETRY_JOURNAL_ENABLED=1
GRIDEX_TELEMETRY_JOURNAL_PATH=/var/lib/gridex/telemetry-journal.ndjson
GRIDEX_TELEMETRY_JOURNAL_MAX_BYTES=4194304
GRIDEX_TELEMETRY_JOURNAL_SECONDS=5
```

`gridex-rockpie.service` owns `/var/lib/gridex` through its systemd
`StateDirectory`. The NDJSON file is fsync'd after each record. Before a write
would exceed the configured bound, the current file becomes `.1`; the former
`.1` is discarded. Therefore the local disk footprint is bounded to
approximately two journal windows. Deployments must choose the capacity based
on the desired offline retention period and expected node count.

Each current record has `schemaVersion`, a local monotonically increasing `sequence`,
UTC observation time, normalized slot, status, fault count and the
latest safe telemetry values. It intentionally excludes endpoint addresses,
credentials, VPN data and control payloads.

The current local `sequence` resets when the process restarts and is not a
backend deduplication key. A future exporter must add the approved durable
`recordId`; it must not claim replay or acknowledgement before that migration.

### ESP32 provisioning and health

IP configuration is DHCP-only on the isolated OT interface. The authoritative
stable address is the protected ROCK Pi DHCP reservation or approved OT DNS
identity; no static node address is compiled into firmware. `GRIDEX_NODE_ENDPOINTS`
on ROCK Pi is deployment configuration and must never be committed with a real
site address.

At local USB serial commissioning, the following command persists logical
identity in `gridex-mbus` NVS:

```text
node <1-247> <type 1-6> <driver-id>
```

Type codes are: `1` inverter, `2` battery/BMS, `3` all-in-one BESS, `4` meter,
`5` EVSE and `6` second cabinet. `status` prints the persisted identity, DHCP
address, Ethernet state, Modbus listener state, bus state, watchdog state and
the trusted ROCK Pi source. `rockpi <IPv4>` remains a separate local command
that restricts Modbus TCP and OTA to that ROCK Pi source.

Persisting a type and driver ID is an inventory/provisioning action only. It
does **not** activate a vendor driver. The current builds deliberately contain
`UnconfiguredDriver`, reject power commands, and do not issue downstream RS485
or CAN requests. A matching signed firmware build and a manufacturer-approved
read map are required before a driver can report ready.

Map version 5 adds the Modbus holding-register health block `0x0046–0x004E`:

| Register | Meaning |
|---|---|
| `0x0046` | Ethernet status (`2` = link/IP available) |
| `0x0047` | Modbus TCP listener status (`1` = listening) |
| `0x0048` | active driver ready (`1` only after a matching driver starts) |
| `0x0049` | device bus initialized |
| `0x004A` | task watchdog enabled |
| `0x004B–0x004D` | total, Ethernet and device-bus recovery counters |
| `0x004E` | last local fault code (`1` Ethernet, `2` bus, `3` watchdog setup) |

The ESP32 task watchdog is fed only after the bounded serial, Ethernet control,
OTA and driver loop all get a scheduling turn. Ethernet IP recovery schedules a
local Modbus listener restart. A compiled hardware driver may opt into the
generic device-bus supervisor; it retries only that driver's local bus every
30 seconds after a reported unhealthy state. The unconfigured driver does not
touch RS485/CAN and therefore does not fabricate a bus-recovery result.

### Verification before deployment

1. Build both ESP32 profiles and the ROCK Pi service.
2. Install the ROCK Pi service package; systemd creates the state directory.
3. Apply only generic journal settings or protected site-local values in
   `/etc/gridex/gridex-rockpie.env`.
4. Flash a reviewed ESP32 image through the approved local process, then use
   local serial `status` and, when appropriate, `node ...`.
5. Confirm ROCK Pi read-only Modbus polling exposes identity, telemetry and the
   `0x0046–0x004E` health range. Do not send a field-device command.
6. Interrupt only the bench node network, confirm a journal transition, then
   restore it and confirm the online transition. Do not use a live BESS for
   this test.

## Български

### Предназначение и граница на безопасност

Услугата на ROCK Pi E поддържа ограничен по размер, устойчив локален telemetry
журнал за ESP32 node slot-овете, които poll-ва. В него се записват периодични
нормализирани snapshots и отделен запис при всяка промяна на polling
състоянието, включително offline, identity и telemetry грешки. Това е функция
само за наблюдение. Тя не преиграва команди, не отваря inbound listener и не
отключва commissioning или device-write gate.

Журналът е нарочно независим от MQTT. Ако Site Router VPN или private broker
не е достъпен, polling-ът и локалното записване продължават. Бъдещ
автентикиран backend recovery worker може да export-ва записи по одобрения v1
договор в `docs/TELEMETRY_JOURNAL_RECOVERY_V1.md`. Този worker, exporter-ът и
acknowledgement механизмът не са част от настоящата промяна; в журнала не се
пазят backend credentials, URL адреси или клиентски данни.

### Локален журнал на ROCK Pi

Защитеният deployment environment приема следните общи настройки:

```ini
GRIDEX_TELEMETRY_JOURNAL_ENABLED=1
GRIDEX_TELEMETRY_JOURNAL_PATH=/var/lib/gridex/telemetry-journal.ndjson
GRIDEX_TELEMETRY_JOURNAL_MAX_BYTES=4194304
GRIDEX_TELEMETRY_JOURNAL_SECONDS=5
```

`gridex-rockpie.service` притежава `/var/lib/gridex` чрез systemd
`StateDirectory`. NDJSON файлът се fsync-ва след всеки запис. Преди записът да
надвиши зададения размер, текущият файл става `.1`, а предишният `.1` се
изтрива. Така заетото дисково пространство остава ограничено приблизително до
два journal прозореца. При deployment капацитетът се избира според желания
offline период и броя нодове.

Всеки текущ запис има `schemaVersion`, локално монотонно растящ `sequence`, UTC време
на наблюдение, нормализиран slot, статус, брой грешки и последните
безопасни telemetry стойности. Умишлено липсват endpoint адреси, credentials,
VPN данни и control payload-и.

Текущият local `sequence` се нулира при рестарт на процеса и не е backend
deduplication ключ. Бъдещ exporter трябва да добави одобрения устойчив
`recordId`; преди тази миграция не бива да се твърди replay или acknowledgement.

### ESP32 provisioning и health

IP конфигурацията е само чрез DHCP по изолирания OT интерфейс. Авторитетният
устойчив адрес е защитеният ROCK Pi DHCP reservation или одобрена OT DNS
идентичност; firmware-ът не съдържа статичен node адрес. `GRIDEX_NODE_ENDPOINTS`
на ROCK Pi е deployment конфигурация и никога не се commit-ва с реален site
адрес.

При local USB serial commissioning следната команда записва логическата
идентичност в NVS `gridex-mbus`:

```text
node <1-247> <type 1-6> <driver-id>
```

Кодовете за type са: `1` инвертор, `2` батерия/BMS, `3` all-in-one BESS, `4`
електромер, `5` EVSE и `6` втори кабинет. `status` показва записаната
идентичност, DHCP адреса, Ethernet състоянието, Modbus listener-а, bus-а,
watchdog-а и доверения ROCK Pi source. `rockpi <IPv4>` остава отделна local
команда, която ограничава Modbus TCP и OTA само до този ROCK Pi source.

Записването на type и driver ID е само inventory/provisioning действие. То не
активира vendor driver. Текущите build-ове умишлено съдържат
`UnconfiguredDriver`, отказват power команди и не подават downstream RS485 или
CAN заявки. Преди driver да отчете ready са нужни съвпадащ signed firmware
build и одобрена от производителя read карта.

Map version 5 добавя Modbus holding-register health блока `0x0046–0x004E`:

| Регистър | Значение |
|---|---|
| `0x0046` | Ethernet статус (`2` = наличен link/IP) |
| `0x0047` | Modbus TCP listener статус (`1` = listening) |
| `0x0048` | active driver ready (`1` само след старт на съвпадащ driver) |
| `0x0049` | device bus е инициализиран |
| `0x004A` | task watchdog е enabled |
| `0x004B–0x004D` | броячи за общо, Ethernet и device-bus възстановяване |
| `0x004E` | последен local fault код (`1` Ethernet, `2` bus, `3` watchdog setup) |

ESP32 task watchdog се подава само след като bounded serial, Ethernet control,
OTA и driver loop получат време за изпълнение. Ethernet IP възстановяването
планира local рестарт на Modbus listener-а. Компилиран hardware driver може да
включи generic device-bus supervisor-а; той опитва само локалния bus на този
driver на всеки 30 секунди след отчетено unhealthy състояние. Unconfigured
driver-ът не докосва RS485/CAN и затова не симулира bus-recovery резултат.

### Проверка преди внедряване

1. Build-ни двата ESP32 профила и ROCK Pi услугата.
2. Инсталирай ROCK Pi service package; systemd създава state directory.
3. Задай само общите journal настройки или защитени site-local стойности в
   `/etc/gridex/gridex-rockpie.env`.
4. Flash-ни прегледан ESP32 образ по одобрения local процес, след което използвай
   local serial `status` и при нужда `node ...`.
5. Потвърди, че read-only Modbus polling-ът на ROCK Pi показва identity,
   telemetry и health range `0x0046–0x004E`. Не изпращай команда към field device.
6. Прекъсни само bench node мрежата, потвърди journal transition, после я
   възстанови и потвърди online transition. Не използвай live BESS за този тест.
