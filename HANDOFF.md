# Handoff — `antouanbg/gridex-edge-gateway`

## English

### Completed in the ROCK Pi E pilot

- Native ARM64 build and both CTest suites passed on the physical board.
- Systemd unit and locked commissioning configuration installed; service is
  disabled and inactive.
- Management Ethernet is working. The OT Ethernet exists but has no carrier;
  no OT address, route or firewall rule has been applied.

### Exact next safe action

Build and flash the tested ESP32-EVB CAN or RS485 profile only after confirming
the free downstream bus. On its local USB serial console, run `rockpi
<ROCK_PI_IPV4>` then `status`. Capture its MAC-independent identity, firmware
version, Ethernet address supplied by the local DHCP server and canonical
Modbus TCP `1502` availability. Do not enable `gridex-rockpie.service`, connect
a BESS, add production addresses or set any `GRIDEX_APPROVE_*` flag.

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

Build-ни и flash-ни тествания ESP32-EVB CAN или RS485 profile само след
потвърждение, че downstream bus е свободен. На local USB serial конзолата
изпълни `rockpi <ROCK_PI_IPV4>`, след това `status`. Запиши MAC-independent
identity, firmware version, Ethernet адреса от local DHCP и наличността на
canonical Modbus TCP `1502`. Не enable-вай `gridex-rockpie.service`, не
свързвай BESS, не добавяй production адреси и не задавай
`GRIDEX_APPROVE_*` flag.

### Остава преди enable на услугата

1. Site-specific OT subnet и physical carrier на втория Ethernet порт.
2. Site Router firewall approval само за backend-to-management Modbus.
3. ESP32 production firmware provisioning, TLS credentials през secret store и
   read-only telemetry soak.
4. Suntech readback и пълният checklist в `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping и frontend backend API.
