# Handoff — GrideX Edge Gateway

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## ROCK system telemetry crash recovery — 2026-09-23

The physical pilot repeatedly exited with `SIGSEGV` shortly after MQTT
connected (`mqtt_connect_result=0`). The earlier "offline MQTT" explanation was
incorrect. The crash location is not yet proven by a core backtrace. The
follow-up change serializes `mosquitto_loop` and MQTT publishing on the service
main thread, includes bounded reconnect attempts, and makes the activation
script verify a stable PID for 45 seconds, covering more than one telemetry
publish interval. If it fails, the script restores
the previous binary/config; if that also fails, it stops the service to avoid
an endless restart loop. This is built and unit-tested on macOS and in an
ARM64 Linux container with `libmosquitto`; it still needs physical validation.
The Site's control approval gates must remain at zero. Do not call the ROCK
system telemetry or OpenRemote/Timescale history live until a fresh MQTT
message and stored datapoint are independently verified. Next safe action:
run `base-rockpie/install/activate-system-telemetry.sh` once on ROCK Pi, then
read service status/logs and verify broker receipt plus the stored datapoint.
If the service fails again, collect a core backtrace before another code fix.

Физическият пилот многократно падна със `SIGSEGV` скоро след успешна MQTT
връзка (`mqtt_connect_result=0`). Предишното обяснение с offline MQTT беше
погрешно. Точното място на crash-а още не е доказано с core backtrace.
Следващата поправка изпълнява `mosquitto_loop` и MQTT публикуването последователно
в основната нишка, добавя ограничени опити за повторна връзка и кара
инсталационния скрипт да проверява стабилен PID за 45 секунди, обхващайки
повече от един интервал за публикуване. При отказ
скриптът възстановява предишния binary/config; ако и той пада, спира услугата,
за да няма безкрайни рестарти. Build и unit тестовете минават на macOS и в
ARM64 Linux контейнер с `libmosquitto`; физическата проверка предстои.
Control approval gate-овете на Обекта остават нула. ROCK системната телеметрия
и OpenRemote/Timescale историята не се обявяват за live преди отделно да се
потвърдят ново MQTT съобщение и записана datapoint стойност. Следващата
безопасна стъпка е еднократно изпълнение на
`base-rockpie/install/activate-system-telemetry.sh` на ROCK Pi, проверка на
service status/log и на broker receipt плюс записаната datapoint стойност.
При нов crash първо се събира core backtrace.

## Pilot inventory reconciled / Пилотен инвентар съгласуван — 2026-09-20

DEPLOYED via supported OpenRemote APIs: pilot Site -> ROCK -> ESP, with the
existing temperature asset reparented under ROCK (same ID/history writer).
All four assets have verified owner links. Owner lacked OR read:assets: granted
that role with restricted_user, NOT unrestricted asset/admin writes. Existing
GrideX administrator membership unchanged. New tokens may be needed to see roles.
Site binding and two gateway bindings are projections of verified OR resources
(migration 009), not independently provisioned inventory. No physical activation,
Ethernet, certificates, MQTT configuration or BESS control changes.
Private backups: inventory-or-XXk1AE before asset creation; inventory-or-ypRcM2
before owner role assignment. Both database dumps passed pg_restore --list;
OR/owner snapshots are private. Final read-back: inventory-or-dHQvbb.
A partial SQL audit failure was corrected; retry reused the same OR IDs.
Eight verification tests + 37 API regression tests PASS; live snapshot validates
hierarchy, owner links, bindings and history writer restricted to its one asset.
Sandbox HTTP tests initially failed EPERM; approved local-port rerun passed.
NOT claimed: owner browser acceptance, physical temperature receipt, or generic
UI/import provisioning enforcement. Those remain pending under the canonical
backend plan. Do not resume local-only bootstrap scripts. Documentation rules
published in backend PR #32, frontend PR #40 and edge PR #20; not merged here.

ВНЕДРЕНО през OpenRemote API: пилотен Обект -> ROCK -> ESP; съществуващият
температурен asset е преместен под ROCK със същия ID/history writer.
Проверени са връзките на четирите assets към собственика. Липсващото OR
read:assets право е добавено с restricted_user, БЕЗ неограничени asset/admin
записи. GrideX администраторското членство е запазено. За новите роли може да
е нужен нов token. Site binding и двата gateway bindings (миграция 009) са
проекции на потвърдени OR ресурси, не отделно провизиран инвентар.
Без физическо активиране, Ethernet, сертификати, MQTT настройки или BESS промени.
Частни backups: inventory-or-XXk1AE преди assets и inventory-or-ypRcM2 преди
owner ролите; двата database dump-а са проверени с pg_restore --list.
OR/owner snapshots са частни; последна проверка inventory-or-dHQvbb.
Поправен е частичен SQL audit отказ; повторението използва същите OR IDs.
8 verification + 37 API regression теста МИНАВАТ; реалният snapshot потвърждава
йерархия, owner links, bindings и writer само до неговия температурен asset.
Първият HTTP тест е блокиран от sandbox EPERM; разрешеното повторение минава.
НЕ са потвърдени: owner browser приемане, физическа температура и универсална
UI/import защита. Те остават задачи по backend плана. Без local-only bootstrap.
Правилата са публикувани в backend PR #32, frontend PR #40 и edge PR #20;
тук не са merge-вани.


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


## CPU temperature publisher / CPU temperature publisher — 2026-09-20

Prepared optional read-only Linux thermal sysfs CPU temperature in existing
ROCK health MQTT payload (`cpuTemperatureC`, Celsius, null if missing/invalid).
Enable only through existing device env: GRIDEX_CPU_TEMPERATURE_ENABLED=1,
GRIDEX_CPU_TEMPERATURE_FILE points to the board-verified CPU thermal zone.
It follows GRIDEX_HEALTH_PUBLISH_SECONDS; no global 15-minute schedule.
Default disabled; no Ethernet, ESP firmware, BESS or command-lock changes.
8/8 native CTests pass with MQTT required. NOT deployed to physical ROCK:
noninteractive SSH refused. Next: approved local build/install with backup,
verify zone/type and service-user read access, enable in existing env, confirm
physical MQTT -> backend outbox -> OpenRemote Timescale -> owner UI.
Backend owner asset/writer/worker configured; synthetic database probe passed,
not physical temperature evidence. Never report CPU as battery temperature.

Готов е optional read-only CPU thermal sysfs сензор в текущия ROCK MQTT health
payload (`cpuTemperatureC`, Celsius, null при липса/грешка). Активиране само през
текущия env: GRIDEX_CPU_TEMPERATURE_ENABLED=1 и GRIDEX_CPU_TEMPERATURE_FILE към
проверен CPU thermal zone. Следва GRIDEX_HEALTH_PUBLISH_SECONDS, не общи 15 минути.
По подразбиране е изключен. Без Ethernet/ESP firmware/BESS/command-lock промени.
8/8 native CTest минават със задължителен MQTT. НЕ е внедрен: SSH е отказан.
Следва одобрен local build/install с backup, zone/type и service-user read
проверка, активиране в текущия env и реален MQTT -> outbox -> Timescale -> UI.
Backend asset/writer/worker са настроени; synthetic DB probe минава, но не
доказва физическа температура. CPU температура не се представя като батерийна.

## Bootstrap path correction / Поправка на bootstrap пътя — 2026-09-20

Physical user run of 123e410 exposed a helper bug: source_dir resolved to repo
root, so only the core test ran and ROCK executable was absent. That run is NOT
successful image/MQTT acceptance. Corrected root to base-rockpie, added explicit
executable gate and --skip-dependencies for retries without apt changes. Two
regression tests execute the real shell helper with fake host tools: exact CMake
source/required MQTT flags/staging and missing-binary rejection. Both pass; shell
syntax passes. Native Linux retry remains pending. Earlier eight native CTests
tested the correct manually chosen source, not the faulty helper path.
The user's dependency run also upgraded OpenSSL packages; no GrideX service
replacement or Ethernet change occurred. Preserve existing live config/keys.

Физическото изпълнение на 123e410 откри грешка: source_dir сочеше repo root;
мина само core тестът и липсваше ROCK executable. Това НЕ е успешно image/MQTT
приемане. Пътят е поправен към base-rockpie, добавени executable gate и
--skip-dependencies за повторение без apt промени. Два regression теста изпълняват
реалния shell helper с fake host tools и проверяват source/MQTT flags/staging и
отказ при липсващ binary; минават, както и shell syntax. Linux повторението
предстои. Старите 8 native CTests бяха с правилно ръчно избран source, не с
грешния helper. Dependency изпълнението е обновило OpenSSL пакети; няма подменена
GrideX услуга или Ethernet промяна. Запазват се live конфигурацията и ключовете.

## Image MQTT bootstrap / Image MQTT подготовка — 2026-09-19

Owner requested dependency provisioning in the image flow instead of manual
commands per board. Added build-image-payload.sh: Debian/Armbian dependency
install including Git/Mosquitto, required-MQTT build, CTest, linkage check and
DESTDIR staging with package versions/binary hash. No live install/restart/network
changes. GRIDEX_REQUIRE_MQTT rejects missing dependencies or disabled MQTT.
Per-device keys/claim remain outside the base image; no cloned credentials.
Validation: shell syntax, native MQTT build and missing-PkgConfig fail-closed check.
Linux bootstrap/ARM64 payload, disk image assembly and first-boot acceptance
remain pending; this is not a generated/flashed image or real MQTT delivery.
See docs/ROCKPI_IMAGE_PROVISIONING.md. Source branch feat/image-mqtt-bootstrap
includes approved transport documentation from PR #18.

Собственикът поиска зависимостите да се осигуряват при image подготовката,
не ръчно за всяка платка. build-image-payload.sh инсталира Debian/Armbian
зависимости с Git/Mosquitto, build със задължителен MQTT, CTest, linkage проверка
и DESTDIR staging с package версии/hash. Без live install/restart/мрежови промени.
GRIDEX_REQUIRE_MQTT отказва липсващи зависимости/изключен MQTT. Per-device keys/
claim остават извън base image. Проверени shell syntax, native MQTT build и
fail-closed при липсващ PkgConfig. Linux bootstrap/ARM64 payload, image assembly
и first-boot приемане предстоят; няма готов/flash-нат имидж или real MQTT receipt.
Виж docs/ROCKPI_IMAGE_PROVISIONING.md; branch включва документацията от PR #18.

## Approved dual transport plan / Одобрен план за два транспорта — 2026-09-19

Owner approval recorded for per-Site WireGuard-private OR direct MQTT-mTLS.
Canonical execution checklist: backend docs/PER_SITE_TRANSPORT_AND_ENROLLMENT.md
on branch docs/per-site-transport. Twelve TODO items cover contract, persistence,
existing broker/worker, ingress, certificate lifecycle, first-boot claim, approved
configuration application, UI, signed firmware, ROCK-initiated ESP OTA, fleet
operations and end-to-end release acceptance. No new menu; no SSH requirement.
This change is documentation only: no listener, runtime env, migration, device
or router changed. Next: versioned transport contract, then persistence/worker.
Heartbeat implementation is merged; physical delivery still needs acceptance.

Записано е одобрение за избор по Обект: WireGuard-private ИЛИ direct MQTT-mTLS.
Каноничният план е backend docs/PER_SITE_TRANSPORT_AND_ENROLLMENT.md в branch
docs/per-site-transport. 12 TODO задачи: договор, база, broker/worker, входове,
сертификати, first-boot claim, одобрено прилагане, UI, подписан firmware, ESP OTA
от ROCK, управление на много обекти и end-to-end приемане. Без ново меню и SSH
зависимост. Само документация: без listener/env/миграция/device/router промени.
Следва versioned transport договор, после база/worker. Heartbeat кодът е слят;
физическата доставка още изисква приемане.

## Device liveness / Жизненост на устройствата — 2026-09-19

Existing periodic ROCK MQTT health is independent of PCS heartbeat/commissioning.
Node payload now carries lastSuccessfulContactAt from successful polling; never
seen is null and failed polls preserve it. Seven native CTests pass, including
payload and TCP polling tests (host-network rerun after sandbox bind denial).
Not installed on physical ROCK. SSH BatchMode denied, agent has no identities.
Backend MQTT is loopback-only; approved private LAN path, mTLS identity/topics,
native ARM64 build/install, backend migration/worker and real browser checks
remain. No device resets, control writes, route changes or VPN activation.
Image ownership/preflight PR #16 remains separate; do not lose those corrections.

Периодичният ROCK MQTT health е отделен от PCS heartbeat/commissioning.
Node payload вече носи lastSuccessfulContactAt от успешен polling; без проба е
null, неуспешните проби го запазват. 7 native CTest теста минават (повторени
извън sandbox TCP bind ограничението). НЕ е инсталирано на физическия ROCK.
SSH BatchMode отказва, agent няма ключове. MQTT е loopback-only; остават частен
LAN път, mTLS identity/topics, ARM64 install, backend migration/worker и browser
проверка. Без reset, control writes, маршрути или VPN. Image поправките от
PR #16 са отделни и трябва да се запазят при бъдещия имидж.
## Image journal preparation / Подготовка на журнала в имиджа — 2026-09-19

Added packaged sysusers/tmpfiles definitions, explicit post-install state
preparation and service-user journal preflight; logs directory/umask are explicit.
Validation: shell syntax, non-root preflight regression tests (preservation,
missing/read-only paths, symlink rejection, disabled journal), CMake configure
and targeted CTest pass on macOS. Linux sysusers/tmpfiles not executed here.
See `docs/ROCKPI_IMAGE_PROVISIONING.md` for new-device enrolment and acceptance.
Owner's local repair yielded three online ESP snapshots with advancing heartbeat;
this is not backend delivery. No image flashed, live unit replaced or VPN enabled.
Next: review PR, Linux sysusers/tmpfiles fresh/restore integration test, then
approved pilot install/reboot/rotation acceptance. Automated claim and backend
heartbeat ingestion/UI remain unfinished. Existing device control locks stay on.

Добавени sysusers/tmpfiles правила, подготовка след инсталация и journal preflight
като service user; изрични logs directory/umask.
Проверки: shell syntax, non-root тестове (запазване, липсващ/read-only път,
symlink отказ, изключен журнал), CMake configure и целевият CTest минават на macOS.
Linux sysusers/tmpfiles не са изпълнявани тук. Новото устройство и приемането
са в `docs/ROCKPI_IMAGE_PROVISIONING.md`. Локалната поправка от собственика даде
три online ESP проби с растящ heartbeat; това не доказва backend доставка.
Без flash, смяна на живата услуга или VPN. Следва PR review, Linux fresh/restore
интеграционен тест, после одобрен пилотен install/reboot/rotation тест. Автоматичният
claim и backend heartbeat/UI остават незавършени. Управлението остава заключено.

## Diagnostic checkpoint — 2026-09-14 / Диагностичен checkpoint

ESP32 is online in three post-USB ROCK Pi slot reads with changing heartbeat.
See [diagnostic caveats](docs/MODBUS_DIAGNOSTIC_CAVEATS.md): a second direct TCP
session is not a reliable health probe. USB may reset the node. Next: inspect
and correct journal write permissions, then verify snapshots/rotation. MQTT
backend receipt and newer firmware health fields remain unverified.

ESP32 е online в три ROCK Pi slot проби след USB с променящ се heartbeat.
Виж [диагностичните особености](docs/MODBUS_DIAGNOSTIC_CAVEATS.md): втора пряка
TCP сесия не е надежден health тест. USB може да рестартира нода. Следва
проверка/корекция на journal правата и snapshots/ротация. MQTT получаване в
backend и health полетата на нов firmware остават непотвърдени.

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
   - Contract draft: `docs/TELEMETRY_JOURNAL_RECOVERY_V1.md`.
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
   - Чернова на договора: `docs/TELEMETRY_JOURNAL_RECOVERY_V1.md`.
   - Следващо действие: създай отделни, координирани PR-и в
     `antouanbg/gridex-edge-gateway` и `antouanbg/gridex-openremote-backend`
     за versioned record identity, export/acknowledgement договора и recovery
     worker-а. Тази възможност днес не е имплементирана.
