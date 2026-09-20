# GrideX Edge Gateway — Engineering Rules

## Strategic invariant: OpenRemote-only inventory / Стратегическо правило — 2026-09-20

Owner-confirmed: OpenRemote is the ONLY authoritative place for all operational
inventory, Sites, devices, gateways, sensors and resource relationships. This
applies equally to user actions through the frontend and Codex/operator actions
under owner instructions: create/provision/update resources through supported
OpenRemote APIs, normally orchestrated by the authorized GrideX backend. Never
bypass OpenRemote by SQL, import, scripts, browser storage or a second registry.
Do not expose administrative credentials in the frontend. No local-only resource
may be presented as provisioned. Require verified OR identity, hierarchy,
owner/realm access and durable bindings before success; outages and partial
failures stay pending/failed and must reconcile idempotently.
Local drafts, delivery queues and disposable read projections are allowed ONLY
as workflow data referencing OR or a pending request, never independent inventory.
Device configuration/NVS and certificates are execution artifacts, not a registry.
Keycloak identity and business records are separate concerns. Anonymous demo
fixtures remain explicitly synthetic, never registered customer/live inventory.
This decision supersedes conflicting older local-only provisioning instructions.
Preserve existing data and safety locks; reconcile legacy orphans with backup,
not blind deletion. Canonical plan: backend docs/OPENREMOTE_PROVISIONING_AUTHORITY.md.
Documentation is not runtime enforcement; migration and acceptance remain pending.

Потвърдено от собственика: OpenRemote е ЕДИНСТВЕНОТО основно място за целия
оперативен инвентар, Обекти, устройства, шлюзове, сензори и ресурсните им връзки.
Правилото важи еднакво за потребителя през frontend и за Codex/оператор по
инструкции на собственика: създаване/провизиране/обновяване през поддържаните
OpenRemote API, обичайно чрез GrideX backend с проверени права. Без заобикаляне
чрез SQL, import, скриптове, browser storage или втори регистър. Без admin тайни
във frontend. Local-only ресурс не се показва като провизиран. Успех изисква
проверени OR идентичност, йерархия, собственик/realm права и устойчив binding;
отказите остават pending/failed и се съгласуват идемпотентно.
Локални чернови, опашки и възстановими проекции за четене са допустими САМО като
данни за процеса с връзка към OR или чакаща заявка, никога независим инвентар.
Device конфигурации/NVS и сертификати са изпълними настройки, не регистър.
Keycloak идентичности и бизнес записи са отделни. Анонимното демо остава ясно
синтетично, не регистриран клиентски/live инвентар.
Решението отменя противоречащи стари инструкции за local-only provisioning.
Пази данните и safety locks; съгласувай наследените записи с backup, без сляпо
изтриване. Каноничен план: backend docs/OPENREMOTE_PROVISIONING_AUTHORITY.md.
Документацията не е runtime защита; миграцията и приемането предстоят.


## Approved per-Site transports / Одобрени транспорти по Обект — 2026-09-19

Owner explicitly approves implementation and publication of both selectable
modes: wireguard_private (ROCK → Site Router → VPN → MQTT) and mqtt_mtls_direct
(ROCK → Internet → controlled MQTT mTLS ingress). This supersedes older blanket
VPN-only/public-MQTT prohibitions for that scoped ingress only. Same identity,
topic ACLs, telemetry/heartbeat contract and Site permissions in both modes.
One active mode per ROCK; no automatic downgrade. Router remains the VPN peer;
ESP/OT/admin/DB remain non-public. No SSH dependency for the intended enrolment
or OTA process. Follow the canonical backend plan
[PER_SITE_TRANSPORT_AND_ENROLLMENT](https://github.com/antouanbg/gridex-openremote-backend/blob/docs/per-site-transport/docs/PER_SITE_TRANSPORT_AND_ENROLLMENT.md).
Do not confuse approval or Git publication with deployed, tested connectivity.
Activation follows its security and commissioning gates; preserve control locks.
UI selection belongs inside existing Site/Devices settings, not a new menu item.

Собственикът изрично одобрява реализация и публикуване на избираемите режими
wireguard_private (ROCK → рутер → VPN → MQTT) и mqtt_mtls_direct (ROCK → Интернет
→ контролиран MQTT mTLS вход). Старите общи VPN-only/public-MQTT забрани се
отменят само за този ограничен вход. Идентичност, topic ACL, heartbeat/telemetry
договор и Site права са еднакви. Един активен режим на ROCK, без автоматичен
downgrade. VPN peer остава рутерът; ESP/OT/admin/DB не стават публични. Целевият
provisioning/OTA процес не зависи от SSH. Следвай каноничния backend план по-горе.
Одобрение/Git публикация не означават внедрена/тествана връзка. Активиране след
security/commissioning gates; control locks се пазят. Изборът е вътре в текущите
настройки Обект/Устройства, не ново меню.


## Modbus diagnostic rule / Правило за Modbus диагностика

Read `docs/MODBUS_DIAGNOSTIC_CAVEATS.md` before investigating ESP32 timeouts.
The current server serves one active TCP session: an additional probe can time
out while ROCK Pi polling works. First read the ROCK Pi normalized node slot
and compare heartbeat and sample age across samples. USB opening may reset the
node; disclose this and do not claim post-USB results prove pre-USB health.
Do not stop polling, flash, reset or change trusted sources merely to diagnose.

Прочети `docs/MODBUS_DIAGNOSTIC_CAVEATS.md` преди ESP32 timeout диагностика.
Текущият server обслужва една активна TCP сесия: допълнителна проба може да
изтече, докато ROCK Pi polling работи. Първо чети нормализирания slot през
ROCK Pi и сравнявай heartbeat и възрастта на данните между пробите. USB
отварянето може да рестартира нода; съобщи това и не представяй резултатите
след USB като доказателство за състоянието преди него. Не спирай polling,
не flash-вай, reset-вай или променяй trusted source само за диагностика.

## Architecture and safety

- The Site Router, not ROCK Pi or ESP32, terminates the WireGuard tunnel.
- ESP32 nodes are on the OT network. ROCK Pi E polls them through Modbus TCP
  and is the sole bridge to private MQTT through the Site Router VPN.
- Do not add WireGuard, public MQTT credentials or direct cloud commands to an
  ESP32 node.
- Vendor maps, signs, scaling and serial/CAN details belong to a device driver.
  Commands must be validated, clamped, time-limited, logged and fail safe.

## ROCK Pi E operating-system baseline

Image builds must use install/build-image-payload.sh and GRIDEX_REQUIRE_MQTT=ON.
Provision Git, compiler, CMake, pkg-config, OpenSSL/CA and libmosquitto dependencies
in the builder; missing MQTT must fail, never silently produce a disabled publisher.
Stage and test before installation. No enrolled keys/configuration in base images.

Image build използва install/build-image-payload.sh и GRIDEX_REQUIRE_MQTT=ON.
Git, compiler, CMake, pkg-config, OpenSSL/CA и libmosquitto се осигуряват от
builder-а; липсващ MQTT спира build-а, не произвежда мълчаливо изключен publisher.
Първо staging и тестове; без заведени ключове/конфигурации в базовия имидж.

Image/install rule (2026-09-19): follow `docs/ROCKPI_IMAGE_PROVISIONING.md`.
Package sysusers/tmpfiles state preparation; run it after install/restore before
service start. Test journal append/rotation as gridex, not root. Preserve data,
never recursively chown backups/secrets, never clone enrolled identities or keys.
Require first-boot and upgrade tests before calling an image ready. Registration,
ESP heartbeat, PCS heartbeat and backend receipt are separate evidence.

Правило за имидж/инсталация: следвай `docs/ROCKPI_IMAGE_PROVISIONING.md`.
Включвай sysusers/tmpfiles подготовката и я изпълнявай след install/restore преди
старт. Тествай запис/ротация като gridex, не root. Без загуба на данни, рекурсивен
chown на backups/тайни или клонирани ключове/идентичности. Изисквай first-boot и
upgrade тестове преди готов имидж. Регистрация, ESP/PCS heartbeat и backend
получаване са отделни доказателства.

The current **pilot** operating-system candidate for the Radxa ROCK Pi E is:

- Armbian 26.8.1 Minimal CLI, Debian 13 (Trixie), ARM64;
- `Armbian_26.8.1_Rockpi-e_trixie_current_6.18.43_minimal.img.xz`;
- SHA256 `85def0ac69ed7f5d1c1a43d6d0830db0ff92ca698b4bb5c62880ee8cad384813`;
- the Armbian `current` / Linux 6.18.43 Rockchip kernel line.
- no desktop environment and no Docker workload on the ROCK Pi E.

This choice is made because the GrideX base service needs CMake 3.20+, a
C++20-capable compiler and current MQTT packages. Armbian lists this ROCK Pi E
minimal image as tested and stable. Do not use the vendor Debian Buster image
as a GrideX production operating system: it is useful only as a
hardware-reference image for vendor dual-Ethernet validation and its userspace
is end of life.

The pilot candidate is **not** a production approval until it passes the
ROCK Pi E acceptance procedure on the physical board. Before a site deployment:

1. record the exact image filename, release, kernel version and SHA256;
2. verify both Ethernet interfaces are present and stable simultaneously;
3. assign WAN/management to the Site Router and a separate static OT interface
   without a default gateway;
4. verify boot, 20 controlled reboots, power-loss recovery and serial console;
5. build and test the native C++20 GrideX service on the board;
6. test Modbus TCP to the BESS, Modbus TCP polling to ESP32 nodes, northbound
   Modbus, and private MQTT through the Site Router VPN;
7. run a 24-hour telemetry soak test with writes disabled.

Pin the accepted image and its package/kernel update policy in repository
documentation after these tests. Never enable an untested kernel upgrade on a
commissioned site. The Site Router remains the sole WireGuard endpoint.

### Initial microSD provisioning procedure

Use this procedure only on a new or explicitly approved removable microSD card.
It is destructive: the selected card is erased completely.

1. Download the pinned image and verify its SHA256 before it is written.
2. Identify the physical removable card by model and capacity using a read-only
   disk listing. Show the exact disk identifier and obtain explicit user
   confirmation before erase/write; never infer the target from its name.
3. Use Balena Etcher on macOS: choose the verified `.img.xz` file, select only
   the confirmed removable card, start Flash, and wait for Etcher validation to
   complete. Etcher handles administrator authorisation locally; never request
   or record that password.
4. Eject the card cleanly, insert it in the ROCK Pi E, and power the board from
   a stable 5 V / 2 A or higher supply. USB computer power is acceptable only
   for a short bench test.
5. Connect LAN to the Site Router, discover the DHCP lease, and use SSH or the
   1,500,000 baud UART console for first boot diagnosis. Change initial
   credentials before installing GrideX services; do not commit credentials or
   obtained device addresses.
6. Keep all BESS/PCS writes locked for the first boot. Complete the hardware
   acceptance procedure above before enabling any command path.

## OLIMEX ESP32-EVB — verified local bench knowledge

The supported node board family is OLIMEX ESP32-EVB / ESP32-EVB-EA-IND.

- PlatformIO environment: `board = esp32-evb`, Arduino framework.
- The USB serial console uses 115200 bps. Keep one serial connection open while
  issuing a bench command: opening and closing the port for each command can
  reset the ESP32 and lose the command.
- Upload at `115200` bps on this setup. The higher 921600 bps upload rate was
  observed to corrupt the transfer after the ESP32 bootloader connected.

### OTA rule / Правило за OTA

- ESP32 OTA is local ROCK Pi → ESP32 Ethernet only. The Site Router controls
  any WireGuard access to the ROCK Pi; it must not forward WireGuard or public
  Internet traffic directly to ESP32.
- OTA must remain disabled until local serial provisioning records a single
  ROCK Pi source address and a per-node secret verifier. Store only a hash on
  the ESP32 and never commit a token, staging image, node address or device
  identity.
- The ROCK Pi OTA client has no listener. It must validate firmware SHA-256,
  use an owner-only token file, and be run only by an approved operator.
- After opening a USB serial session, wait for the ESP32 boot message before
  issuing a provisioning command; opening a serial port can reset the board.

- OTA за ESP32 е само по локален Ethernet път ROCK Pi → ESP32. Site Router
  управлява всеки WireGuard достъп до ROCK Pi; той не трябва да препраща
  WireGuard или публичен Интернет трафик директно към ESP32.
- OTA остава изключено, докато local serial provisioning не запише единствен
  source адрес на ROCK Pi и verifier за отделна тайна на нода. На ESP32 се
  записва само hash; не записвай в Git token, staging image, адрес на нод или
  идентификатор на устройство.
- OTA client-ът на ROCK Pi няма listener. Той валидира SHA-256 на firmware-а,
  използва token файл само за собственика и се изпълнява единствено от одобрен
  оператор.
- След отваряне на USB serial сесия изчакай boot съобщението на ESP32 преди
  provisioning команда; отварянето на serial port може да рестартира платката.

## Relays are out of scope / Релетата не участват в решението

The temporary relay test is complete and has been removed at the owner's request.
Do not add relay commands or automatic relay pulses to EMS firmware.

Временният тест на релетата приключи и е премахнат по искане на собственика.
Не добавяй команди за релета или автоматични импулси в EMS firmware.

## Repository hygiene

- Never commit device-specific credentials, VPN keys, customer addresses,
  production IP ranges, inventory identifiers or USB device paths.
- Never ask a user to send an administrator, SSH or application password in
  chat. When an approved local action needs one, stop at the prompt and ask the
  user to type it directly into the local Terminal or graphical password dialog.
- Maintain English technical documentation with a matching Bulgarian section
  whenever user-facing or operational documentation is changed.
- Record material unfinished deployment work in `HANDOFF.md`, naming this
  repository and the exact next safe action. Update it before ending a
  substantial hardware or deployment task.
- A temporary ESP32 bench connection through the ROCK Pi management Ethernet
  is read-only telemetry testing only. It is never the production OT topology,
  must not be used to reach a BESS or issue device commands, and must be
  removed from the configuration when the isolated OT port is commissioned.
- Before changing firmware, inspect `git status`; preserve unrelated work.

## Node network provisioning / Мрежово provision-ване на нод

- A Site Router DHCP reservation (or an approved OT DNS identity) is the
  authoritative stable address for each ESP32 node. Do not depend on a random
  DHCP lease after reboot.
- Keep deployed `GRIDEX_NODE_ENDPOINTS` only in the root-owned
  `/etc/gridex/gridex-rockpie.env`; Git contains examples and templates only.
- ESP32 NVS stores its trusted `rockpi_ip`, logical node identity and OTA hash
  verifier. These are set locally at commissioning and do not need rewriting
  after a normal reboot.
- When a node address changes, update the Site Router reservation and ROCK Pi
  endpoint configuration in one controlled change, restart only the locked
  read-only service, and verify its normalized slot before any next step.

- DHCP reservation в Site Router (или одобрена OT DNS идентичност) е
  авторитетният устойчив адрес за всеки ESP32 нод. Не разчитай на случаен DHCP
  lease след рестарт.
- Пази внедрените `GRIDEX_NODE_ENDPOINTS` само в root-owned
  `/etc/gridex/gridex-rockpie.env`; Git съдържа само примери и templates.
- ESP32 NVS пази доверения `rockpi_ip`, логическата идентичност на нода и OTA
  hash verifier. Те се задават локално при commissioning и не се презаписват
  при нормален рестарт.
- При промяна на node адрес обнови Site Router reservation и ROCK Pi endpoint
  конфигурацията в една контролирана промяна, рестартирай само заключената
  read-only услуга и провери нормализирания slot преди следваща стъпка.

## Isolated OT DHCP / Изолиран OT DHCP

- gridex-ot-dhcp.service serves DHCP only on the dedicated ROCK Pi OT
  interface. Its interface, range and optional reservations come only from
  the protected deployment environment file.
- Management/WAN retains the sole default route. OT has no default route,
  forwarding or NAT; advertising ROCK Pi as node gateway must not create
  Internet access for OT devices.
- Deployment environment values must be shell-valid `KEY=value` assignments;
  never place angle-bracket placeholders in a deployed environment file.
- A node moved from a bench network to OT must have its trusted ROCK Pi source
  updated locally over USB serial after its OT address is ready; otherwise its
  Modbus TCP source check correctly rejects the new path.
- Never commit actual OT addresses, leases, MAC addresses or customer inventory.

- gridex-ot-dhcp.service раздава DHCP само на отделния ROCK Pi OT интерфейс.
  Интерфейсът, диапазонът и незадължителните reservation-и идват само от
  защитения deployment environment файл.
- Management/WAN пази единствения default route. OT няма default route,
  forwarding или NAT; обявяването на ROCK Pi за node gateway не трябва да
  създава Интернет достъп за OT устройства.
- Deployment environment стойностите трябва да са shell-валидни `KEY=value`
  записи; никога не поставяй placeholders в ъглови скоби във внедрен
  environment файл.
- Нод, преместен от bench мрежа към OT, трябва локално през USB serial да
  обнови trusted ROCK Pi source след готов OT адрес; иначе Modbus TCP source
  проверката правилно отказва новия път.
- Никога не commit-вай реални OT адреси, lease-ове, MAC адреси или клиентски
  inventory.

## Mandatory Pull Request workflow / Задължителен Pull Request процес

- Every completed change set must be committed on a named branch, pushed to
  `origin` and given a Pull Request before it is reported as ready for review.
- The Pull Request targets `main` unless an explicitly documented dependency
  requires another base branch. It must state its scope, tests and any
  commissioning limitations.
- Never merge a Pull Request automatically. Report its URL and wait for the
  project owner's review/merge decision.
- If a Pull Request cannot be created because of permissions or a GitHub error,
  report that exact blocker and keep the branch name and commit SHA in
  `CODEX_STATE.md` and `HANDOFF.md` where applicable.

- Всяка завършена промяна се commit-ва в именуван branch, push-ва се към
  `origin` и получава Pull Request, преди да бъде докладвана като готова за
  review.
- Pull Request-ът е към `main`, освен ако изрично документирана зависимост не
  изисква друга base branch. В него се описват обхватът, тестовете и всички
  commissioning ограничения.
- Pull Request не се merge-ва автоматично. Докладва се URL и се изчаква
  review/merge решение на собственика на проекта.
- Ако Pull Request не може да бъде създаден поради права или GitHub грешка,
  докладвай точното препятствие и запиши branch името и commit SHA в
  `CODEX_STATE.md` и при нужда в `HANDOFF.md`.

## Terminology parity / Терминологична синхронизация

For bilingual operational documentation, keep both sections semantically
identical and use the project terms `Site = Обект`, `Edge gateway = Edge шлюз`,
`self-consumption = собствено потребление` and `EFC = еквивалентни пълни
цикли`. Do not translate code identifiers, protocol names or product brands.

За двуезичната оперативна документация поддържай двата раздела семантично
еднакви и използвай термините `Site = Обект`, `Edge gateway = Edge шлюз`,
`self-consumption = собствено потребление` и `EFC = еквивалентни пълни цикли`.
Не превеждай code identifiers, protocol имена или product brands.

## Verified pilot checkpoint / Потвърден пилотен checkpoint

The following results are verified on the current physical pilot and may be
used as evidence by future work; they are not production approval:

- ROCK Pi E completed a native ARM64 build and both repository CTest suites.
- The installed ROCK Pi service is enabled and active, with all write approval
  gates set to `0` and commissioning locked.
- An OLIMEX ESP32-EVB RS485/Ethernet build was flashed over USB at 115200 bps.
- ROCK Pi performed read-only Modbus TCP identity and telemetry reads from the
  ESP32 over the temporary management-LAN bench path.
- The isolated dual-Ethernet pilot is now verified: management keeps the sole
  default route; a narrowly matched `systemd-networkd` OT override and
  interface-bound DHCP service restore the ESP32 reserved lease. The ESP32
  trusted ROCK Pi OT source is set only through local serial provisioning.
- After this change, the locked ROCK Pi service again reported the normalized
  ESP32 slot online through a read-only Modbus TCP register read.
- Some local execution sandboxes prohibit loopback TCP binds. When native
  Modbus CTest servers fail only with `Operation not permitted` on bind, rerun
  the same suite on an approved host-network execution path before diagnosing
  a code regression.
- The ESP32 driver is still unconfigured. No RS485 downstream command, BESS
  connection or production control action has been enabled.

Тези резултати са потвърдени на текущия физически пилот и могат да се ползват
като доказателство при следваща работа; те не са production одобрение:

- ROCK Pi E е изпълнил native ARM64 build и двата CTest пакета в repository-то.
- Инсталираната ROCK Pi услуга е enabled и active, а всички write approval
  gate-ове са `0` и commissioning е заключен.
- RS485/Ethernet build за OLIMEX ESP32-EVB е flash-нат през USB на 115200 bps.
- ROCK Pi е направил read-only Modbus TCP identity и telemetry четене от ESP32
  през временния management-LAN bench path.
- Изолираният dual-Ethernet пилот вече е потвърден: management пази
  единствения default route; тясно ограничен `systemd-networkd` OT override и
  DHCP услуга само за интерфейса възстановяват резервирания ESP32 lease.
  Довереният ROCK Pi OT source на ESP32 се задава само чрез local serial
  provisioning.
- След тази промяна заключената ROCK Pi услуга отново отчете нормализирания
  ESP32 slot online чрез read-only Modbus TCP register read.
- Някои локални execution sandbox-и забраняват loopback TCP bind. Ако native
  Modbus CTest server-ите паднат само с `Operation not permitted` при bind,
  повтори същия пакет през одобрен host-network execution path, преди да
  диагностицираш code regression.
- ESP32 driver-ът все още е unconfigured. Няма разрешена RS485 downstream
  команда, BESS връзка или production control действие.

Before resuming a hardware task, read `HANDOFF.md`, `CODEX_STATE.md` and
`docs/ROCKPI_E_PILOT_STATUS.md`, then inspect the actual repository state.
Преди продължаване на хардуерна задача прочети `HANDOFF.md`, `CODEX_STATE.md`
и `docs/ROCKPI_E_PILOT_STATUS.md`, след което провери реалното състояние на
repository-то.

## Preserved main-branch rules

No public MQTT listener, direct backend route to OT/BESS, or site-to-site routing is permitted.
Update English and Bulgarian operational text in the same commit with identical meaning.
Every HANDOFF must identify its repository directly below the title.

Не се допуска публичен MQTT, директен backend маршрут към OT/BESS или връзка между обекти.
Обновявай EN и BG оперативните текстове в един commit с еднакъв смисъл.
Всеки HANDOFF посочва repository-то непосредствено под заглавието.

## Local telemetry retention and node health / Локално съхранение на telemetry и node health

- ROCK Pi local telemetry journal is observation-only and bounded. It may store
  normalized snapshots and polling transitions, but never device commands,
  broker credentials, node addresses, VPN data or customer inventory.
- Do not claim journal replay or backend acknowledgement exists until the
  backend recovery worker has been implemented and tested separately.
- ESP32 node addressing is DHCP plus protected deployment configuration. Local
  serial identity provisioning stores only node ID, type and requested driver
  ID; it must not activate a driver or weaken the configured ROCK Pi source
  restriction.
- Map version 5 health registers are read-only. Any future RS485/CAN driver
  must explicitly opt into bus recovery and retain the current command lock
  until its manufacturer-approved map and safety tests are complete.

- Local telemetry журналът на ROCK Pi е само за наблюдение и е ограничен по
  размер. Той може да пази нормализирани snapshots и polling transitions, но
  никога device команди, broker credentials, node адреси, VPN данни или
  клиентски inventory.
- Не твърди, че съществува journal replay или backend acknowledgement, докато
  backend recovery worker-ът не бъде имплементиран и тестван отделно.
- ESP32 node адресирането е DHCP плюс защитена deployment конфигурация. Local
  serial identity provisioning пази само node ID, type и заявен driver ID; не
  трябва да активира driver или да отслабва ограничението за конфигурирания
  ROCK Pi source.
- Health регистрите от map version 5 са read-only. Всеки бъдещ RS485/CAN driver
  трябва изрично да се включи към bus recovery и да запази текущия command lock,
  докато manufacturer-approved картата и safety тестовете му не са завършени.
