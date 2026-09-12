# Handoff — GrideX Edge Gateway

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

### Completed in the ROCK Pi E pilot

- Native ARM64 build and both CTest suites passed on the physical board.
- Systemd unit and locked commissioning configuration installed; service is
  disabled and inactive.
- Management Ethernet is working. The OT Ethernet exists but has no carrier;
  no OT address, route or firewall rule has been applied.
- ESP32-EVB RS485/Ethernet firmware is flashed and ROCK Pi has verified
  read-only identity and telemetry reads through the management bench LAN.
- The ESP32 canonical Modbus TCP map responded to an identity read and a
  telemetry-range read. Its unconfigured-driver state was returned as expected;
  the read values were zero and no downstream RS485 request was issued.
- The ROCK Pi Suntech driver now performs the cumulative-energy registers
  122–125 as one atomic Modbus `0x04` range read and exposes PCS operating
  state through the northbound map. This is code-tested only; it has not been
  connected to a Suntech cabinet in this pilot.

### Current implementation

- `gridex_rockpie_service` now keeps polling the ESP32 canonical maps and
  exposes its normalized Modbus TCP listener when enabled.
- Its private MQTT publisher is outbound-only and TLS-only; it publishes Edge
  health and node telemetry but does not subscribe to MQTT commands.
- The code has local Modbus polling and MQTT payload tests. The physical pilot
  still needs the latest binary, an explicitly configured node endpoint and
  private broker CA/identity before it can publish live health.

### Exact next safe action

PR #3 technical review and relay-test removal are recorded in
[PR3_TECHNICAL_REVIEW.md](docs/PR3_TECHNICAL_REVIEW.md).
Before replacing UnconfiguredDriver, add command lifecycle tests for
disable/reject, TTL expiry, replay and reconnect. The existing command shell
is not acceptance evidence for a live actuator.

Keep the ESP32 bench driver unconfigured and capture the exact Deye test
inverter model/revision, RS485 A/B/GND wiring, unit ID and complete
manufacturer-approved read-register map. Only then create and bench-test a
read-only Deye driver. The known string-inverter power-limit register is not
authorization to enable any command path.
Do not enable `gridex-rockpie.service`, connect a BESS, add production
addresses or set any `GRIDEX_APPROVE_*` flag.

### Remaining before service enablement

1. Site-specific OT subnet and physical carrier on the second Ethernet port.
2. Site Router firewall approval for backend-to-management Modbus only.
3. ESP32 production driver provisioning and a read-only telemetry soak.
   Provision the ROCK Pi private MQTT TLS CA/identity through a secret store
   and deploy the tested service configuration. The ESP32 direct MQTT path is
   disabled.
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
- ESP32 canonical Modbus TCP картата е отговорила на identity read и
  telemetry-range read. Очаквано е върнато unconfigured-driver състояние;
  стойностите са нули и не е отправена downstream RS485 заявка.
- Suntech driver-ът на ROCK Pi вече чете кумулативните energy регистри 122–125
  като един atomic Modbus `0x04` range read и показва PCS operating state през
  northbound картата. Това е само code-tested; в този пилот не е свързван
  Suntech кабинет.

### Текуща имплементация

- `gridex_rockpie_service` вече постоянно poll-ва ESP32 canonical картите и
  предоставя нормализиран Modbus TCP listener, когато е enabled.
- Private MQTT publisher-ът му е само outbound и TLS-only; публикува Edge
  health и node telemetry, но не subscribe-ва MQTT команди.
- Кодът има локални тестове за Modbus polling и MQTT payload-и. Физическият
  пилот все още изисква последния binary, изрично конфигуриран node endpoint и
  private broker CA/identity, преди да публикува live health.

### Точна следваща безопасна стъпка

Техническият review на PR #3 и премахването на relay теста са записани в
[PR3_TECHNICAL_REVIEW.md](docs/PR3_TECHNICAL_REVIEW.md).
Преди замяна на UnconfiguredDriver добави command lifecycle тестове за
disable/reject, TTL, replay и reconnect. Съществуващата command основа
не доказва готовност за управление на реално устройство.

Остави ESP32 bench driver-а unconfigured и запиши точния модел/ревизия на Deye
test инвертора, RS485 A/B/GND wiring, unit ID и пълната manufacturer-approved
read-register карта. Едва тогава се създава и bench-тества read-only Deye
driver. Познатият регистър за power limit при string inverter не е разрешение
за command path. Не enable-вай `gridex-rockpie.service`, не свързвай BESS, не
добавяй production адреси и не задавай `GRIDEX_APPROVE_*` flag.

### Остава преди enable на услугата

1. Site-specific OT subnet и physical carrier на втория Ethernet порт.
2. Site Router firewall approval само за backend-to-management Modbus.
3. ESP32 production driver provisioning и read-only telemetry soak.
   Provision-ни private MQTT TLS CA/identity за ROCK Pi чрез secret store и
   внедри тестваната service конфигурация. Директният MQTT от ESP32 е изключен.
4. Suntech readback и пълният checklist в `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping и frontend backend API.
