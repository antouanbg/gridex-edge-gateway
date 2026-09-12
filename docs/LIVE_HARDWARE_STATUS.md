# Live hardware status / Текущ хардуерен статус

Repository / GitHub: antouanbg/gridex-edge-gateway

## English

Read-only inspection on 2026-09-12, after PR #3 was merged. This is a point-in-time
record, not continuous monitoring. No addresses, identifiers or credentials are stored.

| Component | Observed state |
|---|---|
| ROCK Pi SSH, networkd, resolved, timesyncd, cron, rsyslog | Active/running; clock synchronized |
| ROCK Pi systemd failed units | None |
| gridex-rockpie.service | Installed, disabled, inactive/dead; no running GrideX process |
| ROCK Pi TCP Modbus listener | None; no listener on 1502 |
| Management Ethernet | Link up |
| OT Ethernet | No carrier |
| Mosquitto, Docker, gridex-health, gridex-mqtt units | Not found under these names |
| ESP32 USB console | GrideX status responds; node type and driver ID both zero |
| ESP32 Ethernet Modbus TCP | Three successful FC03 identity/telemetry reads from ROCK Pi, map version 4 |
| ESP32 loop heartbeat | 107 → 109 → 111 across successive samples |
| ESP32 device telemetry | DriverMissing (3), quality 0, zero power/energy; not inverter measurements |
| ESP32 MQTT status | Disconnected |

ROCK Pi resources: approximately 59 °C, 207 MiB RAM used out of 968 MiB,
55 GiB disk available, no swap use. These are inspection samples, not limits.
Only SSH and OS name-resolution listeners were observed.

The installed ROCK Pi binary hash matches the earlier pilot artifact, not a
new deployment of PR #3. The ESP32 status map has no firmware commit field;
its exact running revision cannot be established from this check. No upload,
service start or device write was performed. USB access returned a boot log
and a missing NVS namespace message; subsequent status and Ethernet reads
succeeded. Do not assume serial-port access preserves uptime.

### Remaining

1. Build/deploy the reviewed main revision on both boards with writes locked,
   and record firmware identity for future revision checks.
2. Implement and validate the exact inverter's read-only RS485 driver and wiring.
   The current UnconfiguredDriver cannot collect downstream inverter data.
3. Commission the isolated OT Ethernet link and Site Router firewall.
4. Configure and start sustained ROCK Pi node polling after commissioning;
   current successful reads were diagnostic requests, not a running service.
5. Implement/deploy ROCK Pi MQTT telemetry and health publishing through the
   Site Router VPN, then verify backend ingestion, storage and OpenRemote.
6. Test Suntech readback, reconnect and telemetry soak before approving control.

## Български

Read-only проверка на 2026-09-12 след merge на PR #3. Това е моментен запис,
а не непрекъснат мониторинг. Не се записват адреси, идентификатори или тайни.

| Компонент | Наблюдавано състояние |
|---|---|
| ROCK Pi SSH, networkd, resolved, timesyncd, cron, rsyslog | Active/running; часът е синхронизиран |
| ROCK Pi failed systemd услуги | Няма |
| gridex-rockpie.service | Инсталирана, disabled, inactive/dead; няма работещ GrideX процес |
| ROCK Pi TCP Modbus listener | Няма; порт 1502 не слуша |
| Management Ethernet | Има връзка |
| OT Ethernet | Няма carrier |
| Mosquitto, Docker, gridex-health, gridex-mqtt услуги | Не са намерени под тези имена |
| ESP32 USB конзола | GrideX status отговаря; node type и driver ID са нула |
| ESP32 Ethernet Modbus TCP | Три успешни FC03 identity/telemetry четения от ROCK Pi, карта версия 4 |
| ESP32 heartbeat | 107 → 109 → 111 при последователните прочитания |
| ESP32 device telemetry | DriverMissing (3), quality 0, нулеви power/energy; не са измервания от инвертор |
| ESP32 MQTT | Няма връзка |

ROCK Pi ресурси: около 59 °C, 207 MiB използвана RAM от 968 MiB,
55 GiB свободен диск, без swap използване. Това са моментни стойности, не лимити.
Наблюдавани са само SSH и системни name-resolution listeners.

Hash на инсталирания ROCK Pi binary съвпада с по-ранния пилотен файл, а не с
нов deployment на PR #3. ESP32 status картата няма firmware commit поле;
точната работеща ревизия не може да се установи от тази проверка. Не е правен
upload, старт на услуга или device write. USB достъпът върна boot log и
съобщение за липсващ NVS namespace; последващите status и Ethernet четения
са успешни. Не приемай, че отваряне на серийния порт запазва uptime.

### Остава

1. Build/deploy на прегледаната main версия на двете платки с блокирани записи
   и записване на firmware identity за следващи проверки на ревизията.
2. Имплементация и проверка на read-only RS485 driver за конкретния инвертор
   и неговото окабеляване. UnconfiguredDriver не събира данни от инвертора.
3. Настройка на изолираната OT Ethernet връзка и Site Router firewall.
4. Настройка и старт на постоянния ROCK Pi polling след commissioning;
   успешните текущи четения са диагностични, а не от работеща услуга.
5. Имплементация/deploy на ROCK Pi MQTT telemetry и health през Site Router
   VPN, после проверка на backend ingestion, съхранението и OpenRemote.
6. Suntech readback, reconnect и telemetry soak преди разрешаване на control.
