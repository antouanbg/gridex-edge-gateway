# Handoff — `antouanbg/gridex-edge-gateway`

## English

### Completed in the ROCK Pi E pilot

- Native ARM64 build and both CTest suites passed on the physical board.
- Systemd unit and locked commissioning configuration installed; service is
  disabled and inactive.
- Management Ethernet is working. The OT Ethernet exists but has no carrier;
  no OT address, route or firewall rule has been applied.
- ESP32-EVB RS485/Ethernet firmware is flashed and ROCK Pi has verified
  read-only identity and telemetry reads through the management bench LAN.

### Exact next safe action

Keep the ESP32 bench driver unconfigured and capture the exact Deye test
inverter model, RS485 wiring, unit ID and complete manufacturer-approved
read-register map. Only then create and bench-test a read-only Deye driver.
Do not enable `gridex-rockpie.service`, connect a BESS, add production
addresses or set any `GRIDEX_APPROVE_*` flag.

### Remaining before service enablement

1. Site-specific OT subnet and physical carrier on the second Ethernet port.
2. Site Router firewall approval for backend-to-management Modbus only.
3. ESP32 production driver provisioning, TLS credentials via a secret store
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
- ESP32-EVB RS485/Ethernet firmware е flash-нат и ROCK Pi е потвърдил
  read-only identity и telemetry reads през management bench LAN.

### Точна следваща безопасна стъпка

Остави ESP32 bench driver-а unconfigured и запиши точния модел на Deye test
инвертора, RS485 wiring, unit ID и пълната manufacturer-approved
read-register карта. Едва тогава се създава и bench-тества read-only Deye
driver. Не enable-вай `gridex-rockpie.service`, не свързвай BESS, не добавяй
production адреси и не задавай `GRIDEX_APPROVE_*` flag.

### Остава преди enable на услугата

1. Site-specific OT subnet и physical carrier на втория Ethernet порт.
2. Site Router firewall approval само за backend-to-management Modbus.
3. ESP32 production driver provisioning, TLS credentials през secret store и
   read-only telemetry soak.
4. Suntech readback и пълният checklist в `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping и frontend backend API.
