# Handoff — GrideX Edge Gateway

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

### Completed in the ROCK Pi E pilot

- Native ARM64 build and both CTest suites passed on the physical board.
- Systemd unit and locked commissioning configuration installed; service is
  enabled and active, while all write approval gates remain at `0`.
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
  runs the latest locked read-only service with its listener on loopback and a
  configured temporary bench node. Private broker CA/identity is still needed
  before it can publish live health.
- ROCK Pi initiated a verified ESP32 OTA update through the local Ethernet
  path. The ESP32 accepted a per-node token verifier and image SHA-256,
  restarted, and Modbus TCP recovered. This is local bench evidence only; the
  ESP32 has no WireGuard, public Internet or direct MQTT OTA route.

### Exact next safe action

Follow [node network provisioning](docs/NODE_NETWORK_PROVISIONING.md) to
verify the root-owned `GRIDEX_NODE_ENDPOINTS` entry for the restored stable
ESP32 identity, then restart the locked read-only service and confirm the
normalized slot returns online.

Follow [the local commissioning sequence](docs/LOCAL_COMMISSIONING_SEQUENCE.md)
from its first planned step. The current pilot evidence is preserved in
[live hardware status](docs/LIVE_HARDWARE_STATUS.md). Before replacing
`UnconfiguredDriver`, add command lifecycle tests for disable/reject, TTL
expiry, replay and reconnect. The existing command shell is not acceptance
evidence for a live actuator.

Keep the ESP32 bench driver unconfigured and capture the exact Deye test
inverter model/revision, RS485 A/B/GND wiring, unit ID and complete
manufacturer-approved read-register map. Only then create and bench-test a
read-only Deye driver. The known string-inverter power-limit register is not
authorization to enable any command path.
Do not connect a BESS, add production addresses or set any
`GRIDEX_APPROVE_*` flag. The active service is commissioning-locked and must
remain read-only.

### Remaining before production-control enablement

1. Site-specific OT subnet and physical carrier on the second Ethernet port.
2. Site Router firewall approval for backend-to-management Modbus only.
3. ESP32 production driver provisioning and a read-only telemetry soak.
   Provision the ROCK Pi private MQTT TLS CA/identity through a secret store
   and deploy the tested service configuration. The ESP32 direct MQTT path is
   disabled.
4. Suntech readback and complete checklist in `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping and frontend backend API.
6. Before production ESP32 OTA, approve a Site Router policy that reaches only
   the ROCK Pi management endpoint from the authorised backend peer. Keep the
   ESP32 OTA port off WireGuard/public routes, stage the token owner-only on
   ROCK Pi, and define token rotation/release-signing ownership.

## Български

### Изпълнено в ROCK Pi E пилота

- Native ARM64 build и двата CTest пакета са минали на физическата платка.
- Инсталирани са systemd unit и заключена commissioning конфигурация; услугата
  е enabled и active, а всички write approval gate-ове остават на `0`.
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
  пилот изпълнява последната заключена read-only услуга с listener само на
  loopback и конфигуриран временен bench нод. Private broker CA/identity все
  още е нужен, преди да публикува live health.
- ROCK Pi стартира потвърдено ESP32 OTA обновяване по локалния Ethernet път.
  ESP32 прие verifier за отделен token и image SHA-256, рестартира се и
  Modbus TCP се възстанови. Това е само local bench доказателство; ESP32 няма
  WireGuard, публичен Интернет или direct MQTT OTA маршрут.

### Точна следваща безопасна стъпка

Следвай [мрежово provision-ване на нод](docs/NODE_NETWORK_PROVISIONING.md), за
да провериш root-owned `GRIDEX_NODE_ENDPOINTS` стойността за възстановената
устойчива ESP32 идентичност, след което рестартирай заключената read-only
услуга и потвърди, че нормализираният slot се връща online.

Следвай [последователността за локален commissioning](docs/LOCAL_COMMISSIONING_SEQUENCE.md)
от първата планирана стъпка. Текущите pilot доказателства са запазени в
[текущ хардуерен статус](docs/LIVE_HARDWARE_STATUS.md). Преди замяна на
`UnconfiguredDriver` добави command lifecycle тестове за disable/reject, TTL,
replay и reconnect. Съществуващата command основа не доказва готовност за
управление на реално устройство.

Остави ESP32 bench driver-а unconfigured и запиши точния модел/ревизия на Deye
test инвертора, RS485 A/B/GND wiring, unit ID и пълната manufacturer-approved
read-register карта. Едва тогава се създава и bench-тества read-only Deye
driver. Познатият регистър за power limit при string inverter не е разрешение
за command path. Не свързвай BESS, не добавяй production адреси и не задавай
`GRIDEX_APPROVE_*` flag. Активната услуга е commissioning-locked и трябва да
остане read-only.

### Остава преди enable на production control

1. Site-specific OT subnet и physical carrier на втория Ethernet порт.
2. Site Router firewall approval само за backend-to-management Modbus.
3. ESP32 production driver provisioning и read-only telemetry soak.
   Provision-ни private MQTT TLS CA/identity за ROCK Pi чрез secret store и
   внедри тестваната service конфигурация. Директният MQTT от ESP32 е изключен.
4. Suntech readback и пълният checklist в `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping и frontend backend API.
6. Преди production ESP32 OTA се одобрява Site Router policy, която допуска
   само management endpoint-а на ROCK Pi от оторизирания backend peer. ESP32
   OTA портът остава извън WireGuard/public маршрути, token-ът се подготвя
   owner-only на ROCK Pi и се определя собственик на token rotation/release signing.
