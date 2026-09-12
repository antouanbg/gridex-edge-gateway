# Handoff — `antouanbg/gridex-edge-gateway`

## English

### Completed in the ROCK Pi E pilot

- Native ARM64 build and both CTest suites passed on the physical board.
- Systemd unit and locked commissioning configuration installed; service is
  disabled and inactive.
- Management Ethernet is working. The OT Ethernet exists but has no carrier;
  no OT address, route or firewall rule has been applied.

### Exact next safe action

Use the currently working management Ethernet only to discover the known
ESP32-EVB in read-only bench mode. Capture its MAC-independent identity,
firmware version, Ethernet address supplied by the local DHCP server, canonical
Modbus TCP `1502` availability and telemetry-only MQTT/TLS prerequisites.
Do not enable `gridex-rockpie.service`, connect a BESS, add production
addresses or set any `GRIDEX_APPROVE_*` flag.

### Remaining before service enablement

1. Site-specific OT subnet and physical carrier on the second Ethernet port.
2. Site Router firewall approval for backend-to-management Modbus only.
3. ESP32 production firmware provisioning, TLS credentials via a secret store
   and a read-only telemetry soak.
4. Suntech readback and complete checklist in `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping and frontend backend API.

## Български

### Изпълнено в ROCK Pi E пилота

- Native ARM64 build и двата CTest пакета са минали на физическата платка.
- Инсталирани са systemd unit и заключена commissioning конфигурация; услугата
  е disabled и inactive.
- Management Ethernet работи. OT Ethernet е наличен, но няма carrier; няма
  приложени OT адрес, route или firewall правило.

### Точна следваща безопасна стъпка

Използвай работещия management Ethernet само за read-only bench discovery на
познатия ESP32-EVB. Запиши MAC-independent identity, firmware version,
Ethernet адреса от local DHCP, наличност на canonical Modbus TCP `1502` и
предпоставките за telemetry-only MQTT/TLS. Не enable-вай
`gridex-rockpie.service`, не свързвай BESS, не добавяй production адреси и не
задавай `GRIDEX_APPROVE_*` flag.

### Остава преди enable на услугата

1. Site-specific OT subnet и physical carrier на втория Ethernet порт.
2. Site Router firewall approval само за backend-to-management Modbus.
3. ESP32 production firmware provisioning, TLS credentials през secret store и
   read-only telemetry soak.
4. Suntech readback и пълният checklist в `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping и frontend backend API.
