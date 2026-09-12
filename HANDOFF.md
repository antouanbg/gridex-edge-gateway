# Handoff — GrideX Edge Gateway

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

### Completed in the ROCK Pi E pilot

- Native ARM64 build and both CTest suites passed on the physical board.
- Systemd unit and locked commissioning configuration installed; service is
  enabled and active with a loopback-only listener.
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
- **Deployed on the pilot:** the current merged ROCK Pi source was built
  natively and its five CTest tests passed. `gridex-rockpie.service` is now
  enabled and active with the locked read-only profile. The listener binds only
  to loopback, the configured ESP32 node is online in its normalized slot, and
  the journal reports `commissioning_locked`.
- **Recovery check passed:** an ESP32 reset returned the configured node to
  `online=1`; a controlled ROCK Pi reboot then returned the systemd service,
  loopback-only listener and configured node to their expected read-only state.

### Ordered local commissioning sequence — no backend required

The following sequence is the authoritative order for work possible without a
running GrideX backend, OpenRemote instance or private MQTT broker. Do not skip
a safety gate. A checkmark means code/bench evidence exists; it does not mean
production approval.

1. **[Done] Deploy the current service read-only.** The current source has been
   built natively on ROCK Pi; its service is enabled with an explicit bench-node
   endpoint, loopback northbound listener, no live PCS endpoint and every
   `GRIDEX_APPROVE_*` value at `0`.
2. **[Partially verified] Confirm continuous local polling.** ESP32 reset and
   ROCK Pi reboot recovery are confirmed: the node returned online and the
   service continued normally. Next, unplug/reconnect one node and record the
   transition to `online=false`; one failed node must not block the remaining
   node slots.
3. **[Planned] Commission the two Ethernet roles.** Keep management/WAN behind
   the Site Router. Configure the separate OT interface without a default
   gateway, IP forwarding or WAN-to-OT forwarding; move the temporary bench
   node off management Ethernet when OT carrier is available.
4. **[Planned] Provision ESP32 nodes locally.** Assign a unique local endpoint,
   node role and one compiled driver per device type/brand/model/revision. Keep
   an unknown or unvalidated device on `UnconfiguredDriver` and reject commands.
5. **[Planned] Validate vendor telemetry read-only.** Capture the exact device
   model, revision, serial/CAN/RS485 wiring, unit ID and manufacturer register
   map. Compare sampled values with the local device display before adding any
   write mapping.
6. **[Planned] Add local telemetry retention.** Implement a bounded, disk-backed
   journal on ROCK Pi for node state, heartbeat, power, energy and disconnects.
   It is a local diagnostic/replay buffer, not the future PostgreSQL source of
   record and it must not contain credentials.
7. **[Planned] Add a local commissioning view.** Provide a read-only CLI or
   loopback-only status page using the northbound map: node list, online state,
   driver, heartbeat, quality, power, energy and alarm bits.
8. **[Planned] Run failure and recovery tests.** Test ESP32 power loss, Ethernet
   disconnect, ROCK Pi service restart, ROCK Pi reboot and power-loss recovery.
   Confirm no device command is emitted and the node status is reported stale or
   offline as appropriate.
9. **[Planned] Run a 24-hour read-only soak.** Record uptime, reconnect count,
   polling latency, data-age and memory/temperature. Keep physical controls and
   all vendor writes disabled.
10. **[Later, after local acceptance] Provision backend connectivity.** Add the
    Site Router VPN route/ACL, private broker CA and a per-site client identity;
    then verify the documented outbound-only MQTT health/telemetry topics.
11. **[Later] Connect backend ingestion, PostgreSQL and OpenRemote mappings.**
    Only after the preceding stages can backend alarms, history, assets and
    browser DTOs be commissioned.

The read-only service in step 1 is allowed. What remains prohibited is enabling
a configuration that reaches a live BESS/PCS, setting a write approval flag, or
activating a vendor control path before the corresponding commissioning record.

### Exact next safe action

Perform step 2 of the ordered local commissioning sequence. The service is
enabled only with its loopback-only, node-polling configuration. Do not connect
a BESS/PCS, add production addresses or set any `GRIDEX_APPROVE_*` flag.

The Deye driver remains a step-5 read-only task: first record exact model,
revision, RS485 A/B/GND wiring, unit ID and the manufacturer-approved register
map. The known string-inverter power-limit register is not authorization to
enable any command path.

### Remaining before production-control enablement

1. Site-specific OT subnet and physical carrier on the second Ethernet port.
2. Site Router firewall approval for backend-to-management Modbus only.
3. ESP32 production driver provisioning and a read-only telemetry soak.
   Provision the ROCK Pi private MQTT TLS CA/identity through a secret store
   and add the MQTT-enabled build/configuration when a private broker exists.
   The ESP32 direct MQTT path is disabled.
4. Suntech readback and complete checklist in `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping and frontend backend API.

## Български

### Изпълнено в ROCK Pi E пилота

- Native ARM64 build и двата CTest пакета са минали на физическата платка.
- Инсталирани са systemd unit и заключена commissioning конфигурация; услугата
  е enabled и active с listener само на loopback.
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
- **Внедрено на пилота:** текущият merge-нат ROCK Pi source е изграден native
  и петте CTest теста са успешни. `gridex-rockpie.service` вече е enabled и
  active със заключен read-only профил. Listener-ът е само на loopback,
  конфигурираният ESP32 нод е online в нормализирания си slot, а journal-ът
  отчита `commissioning_locked`.
- **Recovery проверката е успешна:** reset на ESP32 върна конфигурирания нод
  към `online=1`; контролиран reboot на ROCK Pi след това възстанови systemd
  услугата, listener-а само на loopback и конфигурирания нод в очакваното
  read-only състояние.

### Последователност за локален commissioning — без backend

Следната последователност е авторитетният ред за работа, възможна без работещ
GrideX backend, OpenRemote instance или private MQTT broker. Не прескачай
safety gate. Отметката означава code/bench доказателство, а не production
одобрение.

1. **[Готово] Внедри текущата услуга в read-only режим.** Текущият source е
   изграден native на ROCK Pi; услугата е enabled с изричен bench-node endpoint,
   loopback northbound listener, без live PCS endpoint и всички
   `GRIDEX_APPROVE_*` стойности на `0`.
2. **[Частично потвърдено] Потвърди постоянния локален polling.** Reset на
   ESP32 и recovery след ROCK Pi reboot са потвърдени: нодът отново е online,
   а услугата продължава нормално. Следва да изключиш/свържеш един нод и да
   запишеш прехода към `online=false`; един отпаднал нод не трябва да блокира
   останалите node slot-ове.
3. **[Планирано] Commission-ни двете Ethernet роли.** Остави management/WAN
   зад Site Router. Конфигурирай отделния OT интерфейс без default gateway, IP
   forwarding или WAN-to-OT forwarding; премести временния bench нод от
   management Ethernet, когато има OT carrier.
4. **[Планирано] Provision-ни ESP32 нодовете локално.** Задай уникален local
   endpoint, node роля и един compiled driver за device type/brand/model/revision.
   Остави непознато или непотвърдено устройство на `UnconfiguredDriver` и
   отказвай команди.
5. **[Планирано] Валидирай vendor telemetry само за четене.** Запиши точните
   device model, revision, serial/CAN/RS485 wiring, unit ID и manufacturer
   register map. Сравни измерените стойности с local device display, преди да
   се добави write mapping.
6. **[Планирано] Добави local telemetry retention.** Имплементирай ограничен
   disk-backed journal на ROCK Pi за node state, heartbeat, power, energy и
   прекъсвания. Това е local diagnostic/replay buffer, не бъдещият PostgreSQL
   source of record и не съдържа credentials.
7. **[Планирано] Добави local commissioning view.** Направи read-only CLI или
   loopback-only status page през northbound картата: node списък, online state,
   driver, heartbeat, quality, power, energy и alarm bits.
8. **[Планирано] Изпълни тестове за отказ и възстановяване.** Тествай загуба
   на захранване на ESP32, Ethernet disconnect, ROCK Pi service restart, ROCK
   Pi reboot и power-loss recovery. Потвърди, че няма device command и node
   status се отчита като stale или offline според случая.
9. **[Планирано] Изпълни 24-часов read-only soak.** Запиши uptime, reconnect
   count, polling latency, data-age и memory/temperature. Остави physical
   control-ите и всички vendor write операции изключени.
10. **[По-късно, след local acceptance] Provision-ни backend connectivity.**
    Добави Site Router VPN route/ACL, private broker CA и client identity за
    всеки Обект; после потвърди описаните outbound-only MQTT health/telemetry
    topics.
11. **[По-късно] Свържи backend ingestion, PostgreSQL и OpenRemote mappings.**
    Едва след предходните етапи могат да се commission-нат backend alarms,
    history, assets и browser DTOs.

Read-only услугата от стъпка 1 е разрешена. Забранено остава enable на
конфигурация, която достига жив BESS/PCS, задаването на write approval flag или
активирането на vendor control path преди съответния commissioning запис.

### Точна следваща безопасна стъпка

Изпълни стъпка 2 от последователността за local commissioning. Услугата е
enabled само с loopback-only node-polling конфигурацията. Не свързвай BESS/PCS,
не добавяй production адреси и не задавай `GRIDEX_APPROVE_*` flag.

Deye driver-ът остава read-only задача от стъпка 5: първо запиши точния
model, revision, RS485 A/B/GND wiring, unit ID и manufacturer-approved register
map. Познатият string-inverter power-limit регистър не е разрешение за
активиране на command path.

### Остава преди enable на production control

1. Site-specific OT subnet и physical carrier на втория Ethernet порт.
2. Site Router firewall approval само за backend-to-management Modbus.
3. ESP32 production driver provisioning и read-only telemetry soak.
   Provision-ни private MQTT TLS CA/identity за ROCK Pi чрез secret store и
   добави MQTT-enabled build/configuration, когато private broker е наличен.
   Директният MQTT от ESP32 е изключен.
4. Suntech readback и пълният checklist в `docs/COMMISSIONING.md`.
5. Backend MQTT ingestion/OpenRemote asset mapping и frontend backend API.
