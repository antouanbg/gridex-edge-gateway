# Local commissioning sequence / Последователност за локален commissioning

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

This is the required order for work that can proceed without a running GrideX
backend, OpenRemote instance or private MQTT broker. A completed item is bench
evidence, not production approval. Do not enable a live BESS/PCS path or any
`GRIDEX_APPROVE_*` value during these stages.

1. **Done — locked read-only service.** Build the current ROCK Pi service,
   enable its systemd unit, keep the northbound listener loopback-only, and
   retain every write gate at `0`.
2. **Done for one node — local polling and recovery.** Verify ESP32 reset,
   Ethernet disconnect/reconnect and ROCK Pi reboot; the node must recover from
   offline to online without a field-device command. Repeat multi-node
   isolation when a second node is available.
3. **Planned — separate the two Ethernet roles.** Keep management/WAN behind
   the Site Router; configure the OT interface without default gateway, IP
   forwarding or WAN-to-OT forwarding. Move the bench node to OT only after
   physical carrier and firewall review.
4. **Planned — provision each node locally.** Give every node one endpoint,
   role and compiled driver for exactly one device type, brand, model and
   protocol revision. Unknown devices remain on `UnconfiguredDriver`.
5. **Planned — validate vendor telemetry read-only.** Record device model,
   revision, wiring, unit ID and manufacturer map; compare samples with the
   local display before creating any write map.
6. **Planned — bounded local retention and commissioning view.** Add a
   credential-free disk journal plus a loopback-only view/CLI for node state,
   heartbeat, quality, power, energy and alarms.
7. **Planned — repeat failure and soak tests.** Include power loss, service
   restart, reboot and 24-hour read-only soak; record uptime, reconnects,
   polling latency, data age, memory and temperature.
8. **Later — backend connectivity.** After local acceptance, approve the Site
   Router ACL and provision private broker CA/client identity. Verify only the
   documented outbound MQTT health and telemetry topics.
9. **Later — backend assets.** Then commission GrideX ingestion, PostgreSQL,
   OpenRemote mappings, alarms, history and frontend DTOs.
10. **Separately — production OTA.** Approve the Site Router policy that can
    reach ROCK Pi only, define release signing and secret rotation ownership,
    and complete an isolated OT OTA soak. ESP32 remains off VPN/public routes.

## Български

Това е задължителният ред за работа, възможна без работещ GrideX backend,
OpenRemote instance или private MQTT broker. Готова точка означава bench
доказателство, а не production одобрение. В тези етапи не разрешавай жив
BESS/PCS път или `GRIDEX_APPROVE_*` стойност.

1. **Готово — заключена read-only услуга.** Изгради текущата ROCK Pi услуга,
   enable-ни systemd unit-а, остави northbound listener-а само на loopback и
   всеки write gate на `0`.
2. **Готово за един нод — локален polling и recovery.** Провери reset на
   ESP32, Ethernet disconnect/reconnect и ROCK Pi reboot; нодът трябва да се
   възстанови offline → online без field-device команда. Повтори multi-node
   изолацията, когато има втори нод.
3. **Планирано — раздели двете Ethernet роли.** Остави management/WAN зад
   Site Router; конфигурирай OT интерфейса без default gateway, IP forwarding
   или WAN-to-OT forwarding. Премести bench нода към OT само след physical
   carrier и firewall review.
4. **Планирано — provision-ни всеки нод локално.** Всеки нод получава един
   endpoint, роля и compiled driver за точно един device type, brand, model и
   protocol revision. Непознатите устройства остават на `UnconfiguredDriver`.
5. **Планирано — валидирай vendor telemetry read-only.** Запиши device model,
   revision, wiring, unit ID и manufacturer map; сравни пробите с local display
   преди да се създаде write map.
6. **Планирано — ограничено local retention и commissioning view.** Добави
   credential-free disk journal и loopback-only view/CLI за node state,
   heartbeat, quality, power, energy и alarms.
7. **Планирано — повтори failure и soak тестовете.** Включи power loss, service
   restart, reboot и 24-часов read-only soak; запиши uptime, reconnects,
   polling latency, data age, memory и temperature.
8. **По-късно — backend connectivity.** След local acceptance одобри Site
   Router ACL и provision-ни private broker CA/client identity. Провери само
   описаните outbound MQTT health и telemetry topics.
9. **По-късно — backend Assets.** След това commission-ни GrideX ingestion,
   PostgreSQL, OpenRemote mappings, alarms, history и frontend DTOs.
10. **Отделно — production OTA.** Одобри Site Router policy, която достига
    само ROCK Pi, определи собственик на release signing и secret rotation и
    изпълни изолиран OT OTA soak. ESP32 остава извън VPN/public маршрути.
