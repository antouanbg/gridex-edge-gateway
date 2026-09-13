# Handoff — GrideX Edge Gateway

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

### Completed in the ROCK Pi E pilot

- Native ARM64 build and both CTest suites passed on the physical board.
- Systemd unit and locked commissioning configuration installed; service is
  enabled and active, while all write approval gates remain at `0`.
- Management Ethernet is working and remains the sole default route. The OT
  Ethernet is isolated with a static address from the protected deployment
  environment; its interface-bound DHCP service issued the ESP32 reservation.
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
- The bounded, fsync-backed local NDJSON telemetry journal is installed on the
  physical ROCK Pi. It records normalized node snapshots and polling-state
  transitions only; it is independent of MQTT and has no replay/control path.
  At the deployment checkpoint the ESP32 node was offline, so the journal
  transition and snapshot must still be verified after the node returns.
- The next ESP32 firmware build adds local logical node provisioning, map-v5
  health registers, task-watchdog status and recovery accounting. It is built
  for CAN and RS485 profiles, but is not yet flashed to the physical pilot.
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
- A repeat OTA revalidation completed on 2026-09-12. The ESP32 accepted the
  verified image, then returned to normal boot mode; its local endpoints became
  reachable and the ROCK Pi normalized polling slot returned online. The
  commissioning lock and all write approval gates remained at `0`.

### Current validated state

The node network recovery is complete: the protected endpoint setting was
verified, the locked read-only service was restarted and its normalized node
slot is online. The local listener and OTA client also passed non-mutating
readiness checks. See [node network provisioning](docs/NODE_NETWORK_PROVISIONING.md).

### Local telemetry deployment checkpoint

- The ROCK Pi service and OT DHCP service are enabled and active after the
  telemetry deployment; the rollback copy remains on the device.
- All control approval gates remain locked. No field-device command, MQTT
  credential or endpoint configuration was changed by this deployment.
- The ESP32 was offline at the post-install check. Consequently, no live
  node-health sample has been accepted as evidence yet. The protected local
  journal was subsequently confirmed to contain records. Restore the node,
  verify the protected endpoint setting locally, then confirm an
  offline-to-online journal transition and a periodic snapshot.

### Validated OT networking pilot

The physical second Ethernet link is active. Static OT addressing and the
interface-bound DHCP service are deployed from the protected deployment
environment. The ESP32 received its reserved OT lease; its local trusted ROCK
Pi source was provisioned over USB serial, and ROCK Pi polling returned the
normalized node slot online. Management remains the sole default route and
every control gate remains locked.

### OT deployment lessons

- Use only shell-valid deployment environment assignments, without
  angle-bracket placeholders.
- Preserve vendor networking files; the dedicated native OT network match
  intentionally takes precedence over their wildcard DHCP rule.
- If an ESP32 is moved to OT, update its trusted ROCK Pi source over local USB
  serial before expecting Modbus TCP polling or local OTA to recover.

### Review artifact

Pull Request #12 for the isolated OT DHCP pilot was merged to `main` at
`6d79295`. Pull Request #13 merged the local journal and ESP32
provisioning/health implementation to `main`; its physical deployment is now
in progress and remains read-only.

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

1. Repeat the protected-environment OT configuration and read-only recovery
   test during each site commissioning; do not place site addresses or MACs in
   Git.
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
7. **Journal recovery export and backend ingestion / Изнасяне на journal-а и backend ingestion**
   - Dependency: a versioned Edge-to-backend export and acknowledgement
     contract, private MQTT TLS identity, and the separate GrideX PostgreSQL/
     Timescale service in `antouanbg/gridex-openremote-backend`.
   - Acceptance: a future ROCK Pi exporter transmits only authenticated,
     normalized journal records through the Site Router VPN; a future backend
     worker persists them idempotently before acknowledging delivery. It does
     not expose the journal filesystem, route OT/BESS to the backend, or send
     control commands.
   - Next action: create separate, coordinated PRs in
     `antouanbg/gridex-edge-gateway` and `antouanbg/gridex-openremote-backend`
     for the versioned record identity, export/acknowledgement contract and
     recovery worker. This capability is not implemented today.

## Български

### Изпълнено в ROCK Pi E пилота

- Native ARM64 build и двата CTest пакета са минали на физическата платка.
- Инсталирани са systemd unit и заключена commissioning конфигурация; услугата
  е enabled и active, а всички write approval gate-ове остават на `0`.
- Management Ethernet работи и остава с единствения default route. OT Ethernet
  е изолиран със статичен адрес от защитения deployment environment; DHCP
  услугата само за този интерфейс издаде ESP32 reservation.
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
- Ограниченият, fsync-backed local NDJSON telemetry журнал е инсталиран на
  физическия ROCK Pi. Той записва само нормализирани node snapshots и
  polling-state transitions, независим е от MQTT и няма replay/control път.
  При checkpoint-а на внедряването ESP32 нодът беше offline, затова transition
  и snapshot от журнала още трябва да се потвърдят след възстановяването му.
- Следващият ESP32 firmware build добавя local logical node provisioning,
  map-v5 health регистри, task-watchdog статус и recovery броячи. Build-нат е
  за CAN и RS485 профилите, но още не е flash-нат на физическия пилот.
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
- Повторната OTA проверка приключи на 2026-09-12. ESP32 прие проверения образ,
  след което се върна в нормален boot режим; локалните му endpoint-и станаха
  достъпни и нормализираният polling slot на ROCK Pi се върна online.
  Commissioning lock и всички write approval gate-ове останаха `0`.

### Текущо валидирано състояние

Възстановяването на мрежовата конфигурация на нода е приключено: защитената
endpoint настройка е проверена, заключената read-only услуга е рестартирана и
нормализираният node slot е online. Локалният listener и OTA client-ът също
минаха проверки без промяна на състояние. Виж
[мрежово provision-ване на нод](docs/NODE_NETWORK_PROVISIONING.md).

### Checkpoint на local telemetry внедряването

- ROCK Pi услугата и OT DHCP услугата са enabled и active след telemetry
  внедряването; rollback копието остава на устройството.
- Всички control approval gate-ове остават заключени. С това внедряване не са
  променяни field-device команда, MQTT credential или endpoint конфигурация.
- ESP32 беше offline при post-install проверката. Затова все още няма приета
  като доказателство live node-health проба. Защитеният local journal беше
  потвърден впоследствие, че съдържа записи. Върни нода, провери локално
  protected endpoint настройката и потвърди offline-to-online journal
  transition и периодичен snapshot.

### Проверен пилот за OT мрежата

Физическият втори Ethernet линк е активен. Статичното OT адресиране и DHCP
услугата, ограничена до този интерфейс, са внедрени от защитения deployment
environment. ESP32 получи резервирания OT lease; довереният ROCK Pi source е
provision-нат през USB serial, а ROCK Pi polling върна нормализирания node slot
в online. Management остава с единствения default route, а всички control
gate-ове остават заключени.

### Уроци от OT внедряването

- Използвай само shell-валидни deployment environment записи, без placeholders
  в ъглови скоби.
- Запази vendor мрежовите файлове; отделният native OT network match умишлено
  има приоритет над wildcard DHCP правилото им.
- Ако ESP32 се премести в OT, обнови trusted ROCK Pi source през local USB
  serial, преди да очакваш възстановяване на Modbus TCP polling или local OTA.

### Артефакт за review

Pull Request #12 за isolated OT DHCP пилота е слят към `main` на `6d79295`.
Pull Request #13 е слял local journal и ESP32 provisioning/health
имплементацията към `main`; физическото ѝ внедряване е в ход и остава
read-only.

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

1. Повтори OT конфигурацията от защитения environment и read-only recovery
   теста при commissioning на всеки site; не записвай site адреси или MAC в
   Git.
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
7. **Изнасяне на journal-а и backend ingestion / Journal recovery export and backend ingestion**
   - Зависимост: versioned Edge-to-backend export и acknowledgement договор,
     private MQTT TLS identity и отделната GrideX PostgreSQL/Timescale услуга в
     `antouanbg/gridex-openremote-backend`.
   - Приемане: бъдещ ROCK Pi exporter изпраща само удостоверени,
     нормализирани journal записи през Site Router VPN; бъдещ backend worker ги
     записва idempotent преди да потвърди доставката. Той не излага journal
     файловата система, не route-ва OT/BESS към backend и не изпраща control
     команди.
   - Следващо действие: създай отделни, координирани PR-и в
     `antouanbg/gridex-edge-gateway` и `antouanbg/gridex-openremote-backend`
     за versioned record identity, export/acknowledgement договора и recovery
     worker-а. Тази възможност днес не е имплементирана.
