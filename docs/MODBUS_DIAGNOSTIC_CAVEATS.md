# ESP32 Modbus diagnostic caveats

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

### Observation — 2026-09-14

A second TCP connection to ESP32 port 1502 succeeded, but its FC03 read timed
out. ROCK Pi already held an established polling session. Source inspection of
`node-esp32-evb/src/EthernetControlServer.cpp` shows that `acceptClient()`
returns while the existing client remains connected; a second socket is not
serviced concurrently. This is a diagnostic limitation, not sufficient evidence
of an Ethernet fault or a failed primary polling session.

USB status showed the trusted source matched the actual ROCK Pi OT address.
Three subsequent FC04 reads of the ROCK Pi normalized slot reported online,
with heartbeat values 23658, 23660 and 23662. Heartbeat is polled via Modbus;
it is not an unsolicited packet stream to the diagnostic client. Backend MQTT
receipt was not verified.

A boot message appeared when USB was opened: a reset may have occurred.
Consequently these results prove post-USB communication only; they do not
conclusively establish the cause of the earlier timeout or pre-USB health.
No firmware upload, provisioning change or control write was issued.

### Procedure for future investigations

1. Read the current handoff and deployed firmware contract before probing.
2. Inspect ROCK Pi service state and existing TCP sessions without stopping them.
3. Read its normalized node slot (FC04, slot base 0x0100); check online,
   changing heartbeat and sample age across multiple samples. Use the matching
   register map; do not assume newer map-v5 fields exist on older firmware.
4. Do not label a node offline based solely on a second direct-session timeout.
   Do not weaken source restrictions to make a diagnostic connection succeed.
5. If USB status is necessary, use 115200 and one persistent session. Record
   any boot message and separate pre/post-USB observations.
6. This does not prove downstream RS485/BESS telemetry, MQTT delivery or OTA.
   Each requires its own evidence. All control gates remain locked.

Separate unresolved issue: ROCK Pi journal writes fail; the service runs as
`gridex`, while the observed journal is root-owned 0640 under a root-owned
0755 directory. Inspect the configured path and systemd write restrictions
before a narrowly scoped ownership fix; then verify snapshots and rotation.
The disk had free space. No ownership fix was made during this inspection.
Actual addresses, USB paths and hardware identifiers are intentionally omitted.

## Български

### Наблюдение — 2026-09-14

Втора TCP връзка към ESP32 порт 1502 се установи, но FC03 четенето изтече.
ROCK Pi вече държеше polling сесия. В
`node-esp32-evb/src/EthernetControlServer.cpp`, `acceptClient()` се връща,
докато настоящият клиент е свързан; втори socket не се обслужва едновременно.
Това е ограничение на диагностиката, не достатъчно доказателство за Ethernet
повреда или отказ на основния polling.

USB статусът показа trusted source, съвпадащ с действителния OT адрес на ROCK
Pi. Три последващи FC04 четения на нормализирания slot върнаха online и
heartbeat 23658, 23660, 23662. Heartbeat се чете чрез Modbus, не е самостоятелен
поток от пакети към диагностичния клиент. MQTT получаването в backend не е
потвърдено.

При USB отварянето се появи boot съобщение: възможен е reset. Резултатите
доказват комуникация след USB, но не установяват категорично причината за
предишния timeout или състоянието преди USB. Няма firmware upload,
provisioning промяна или control write.

### Процедура за бъдещи проверки

1. Прочети актуалния handoff и договора на внедрения firmware преди проби.
2. Провери ROCK Pi услугата и TCP сесиите, без да ги спираш.
3. Чети нормализирания node slot (FC04, начало 0x0100); сравни online,
   променящ се heartbeat и възраст на данните в няколко проби. Ползвай
   съответната карта; не приемай, че стар firmware има map-v5 полета.
4. Не обявявай нода offline само по timeout на втора директна сесия.
   Не отслабвай source ограниченията за диагностична връзка.
5. При необходим USB status ползвай 115200 и една постоянна сесия. Отбелязвай
   boot съобщенията и разделяй наблюденията преди/след USB.
6. Това не доказва downstream RS485/BESS telemetry, MQTT доставка или OTA.
   За всяко трябват отделни доказателства. Control gate-овете остават заключени.

Отделен нерешен проблем: journal записите на ROCK Pi се провалят. Услугата
работи като `gridex`, наблюдаваният журнал е root-owned 0640 под root-owned
0755 папка. Провери конфигурирания path и systemd write ограниченията преди
ограничена ownership корекция; после провери snapshots и ротация. Дискът
имаше свободно място. При проверката не са променяни права. Реални адреси,
USB пътища и хардуерни идентификатори умишлено не са включени.
