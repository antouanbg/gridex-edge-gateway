# CODEX_STATE.md

## ROCK Pi pilot stopped after repeat crash / ROCK Pi пилотът е спрян след повторен crash — 2026-09-23

The physical activation of the serialized MQTT loop (`927d73a`) compiled but
failed the 45-second process-stability check. Rollback restored the previous
binary/config; the previous binary also failed, so the service was stopped.
The one-shot `gdb` stack of that **old binary** shows SIGSEGV in
`libmosquitto.so.1` during `mosquitto_publish_v5()`, called from
`MqttHealthPublisher::publishNodeTelemetry()`, with a separate Mosquitto loop
thread active. This does not diagnose the newer candidate's failure. The
updated one-shot script now builds and runs the candidate under `gdb` from a
separate debug binary without replacing the installed service, then leaves
the service stopped. The first attempt (`de9f42d`) hit `Permission denied`
executing from `/run`, so the debug binary was moved to a separate path in
`/usr/local/bin`; candidate crash evidence is still pending. No live ROCK
telemetry or history datapoint is verified.
The `fc1c012` candidate also crashed, in main-thread `mosquitto_loop()`.
Review found a mismatched class layout across translation units because the
public header was conditional on a PRIVATE compile definition, plus an
unsupported `mosquitto_connect_async()` + manual `mosquitto_loop()` pairing.
Both are corrected on the branch with an ABI regression test. Eight macOS and
eight isolated ARM64 Linux tests pass; physical validation is still pending. Do not activate the service
until the corrected candidate survives the one-shot capture.

Физическата активация на последователния MQTT loop (`927d73a`) се компилира,
но не издържа 45-секундната проверка. Rollback върна предишния binary/config;
и той падна, затова услугата беше спряна. Еднократният `gdb` stack на
**стария binary** показва SIGSEGV в `libmosquitto.so.1` при
`mosquitto_publish_v5()`, извикан от `MqttHealthPublisher::publishNodeTelemetry()`,
докато отделна Mosquitto loop нишка работи. Това не диагностицира отказа на
новия кандидат. Обновеният еднократен скрипт компилира и пуска кандидата под
`gdb` като отделен debug binary, после оставя услугата спряна.
Първият опит (`de9f42d`) получи `Permission denied` при изпълнение от `/run`,
затова debug binary е преместен на отделен път в `/usr/local/bin`, без да
замества инсталирания service binary. Stack-ът на кандидата още предстои.
Няма потвърдена live ROCK телеметрия или history datapoint.
Кандидатът `fc1c012` също падна — в `mosquitto_loop()` на основната нишка.
Открити са различен размер на класа между translation units заради PRIVATE
compile flag и неподдържаната комбинация `mosquitto_connect_async()` + ръчен
`mosquitto_loop()`. И двете са поправени в branch-а с ABI регресионен тест.
Осемте macOS и осемте изолирани ARM64 Linux теста минават; физическата проверка предстои. Не активирай
услугата преди поправеният кандидат да издържи еднократния тест.

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


2026-09-20: optional CPU temperature publisher prepared on feat/rock-temperature;
8/8 CTests pass. Not installed on ROCK (SSH denied); device env activation and
real sensor/MQTT/Timescale/UI verification pending. See newest HANDOFF.
2026-09-20: optional CPU temperature publisher е готов, 8/8 CTest минават.
Не е качен на ROCK (SSH отказан); env активиране и реален сензор/MQTT/Timescale/UI
тест предстоят. Виж най-новия HANDOFF.

2026-09-20: fixed image helper source root after physical failure of 123e410;
two real-helper/mock-tools regressions pass. Retry with --skip-dependencies.
Physical successful payload still unverified; prior core-only run did not qualify.
2026-09-20: поправен source root след физическия отказ на 123e410; два helper
regression теста минават. Повтори с --skip-dependencies. Успешен physical payload
още не е доказан; предходният core-only build не е приемателен тест.

Latest: feat/image-mqtt-bootstrap automates build dependencies (including Git),
required MQTT, tests and image payload staging. Missing MQTT fails closed.
No physical deployment/image flash. Linux/ARM64/first-boot gates remain pending.
Последно: feat/image-mqtt-bootstrap автоматизира зависимости (включително Git),
задължителен MQTT, тестове и image payload staging. Без MQTT build отказва.
Без физическо внедряване/flash; Linux/ARM64/first-boot тестовете предстоят.

2026-09-19: per-Site private WireGuard and direct MQTT-mTLS explicitly approved.
Execution plan recorded in backend docs/PER_SITE_TRANSPORT_AND_ENROLLMENT.md;
12 TODO items, documentation-only publication, no live activation. See HANDOFF.

2026-09-19: изрично одобрени WireGuard-private и direct MQTT-mTLS по Обект.
Планът е в backend docs/PER_SITE_TRANSPORT_AND_ENROLLMENT.md; 12 TODO задачи,
само документална публикация, без live активиране. Виж HANDOFF.

Latest: feat/device-heartbeat adds actual successful node polling time to MQTT;
existing ROCK health publication retained. Seven CTests pass. Not deployed;
SSH authority/private transport and end-to-end backend/UI verification pending.

Последно: feat/device-heartbeat добавя реалното време за успешен node polling
в MQTT; ROCK health се запазва. 7 CTest минават. Не е внедрено; остават SSH,
частен транспорт и end-to-end backend/UI проверка. Виж последния HANDOFF.
2026-09-19: image journal ownership/preflight implementation on
`fix/rockpi-image-journal`; see newest HANDOFF and ROCKPI_IMAGE_PROVISIONING.
No live deployment/image build. Linux tmpfiles and physical reboot/rotation
acceptance pending; no automatic merge. Local ESP evidence is owner-supplied.

2026-09-19: подготовка на journal права/preflight в `fix/rockpi-image-journal`.
Виж HANDOFF и ROCKPI_IMAGE_PROVISIONING. Без живо внедряване/имидж; Linux tmpfiles
и физически reboot/rotation приемане предстоят. Без автоматичен merge.
Локалното ESP доказателство е предоставено от собственика.

## Latest checkpoint — 2026-09-14 / Последен checkpoint

Documentation-only task: recorded single-session Modbus diagnostic limitation
and USB reset uncertainty in AGENTS.md, HANDOFF.md and
docs/MODBUS_DIAGNOSTIC_CAVEATS.md. Branch: docs/modbus-diagnostic-caveat.
Three post-USB online/advancing-heartbeat reads verified; old offline status
below is historical. Next: journal permission diagnosis/fix with snapshot and
rotation checks; MQTT backend receipt still unverified. No runtime changes.

Само документация: single-session Modbus ограничението и USB reset
неопределеността са записани в AGENTS.md, HANDOFF.md и
docs/MODBUS_DIAGNOSTIC_CAVEATS.md. Branch: docs/modbus-diagnostic-caveat.
Три post-USB online/нарастващ heartbeat проби са потвърдени; старият offline
статус по-долу е исторически. Следва journal права и snapshots/ротация;
MQTT backend получаване още не е потвърдено. Без runtime промени.

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Validate the read-only physical deployment of the local telemetry journal.
Pull Request #13 is merged; the ROCK Pi service is installed and active, while
the ESP32 is currently offline. Production control remains out of scope.
Потвърди read-only физическото внедряване на local telemetry журнала. Pull
Request #13 е слят; ROCK Pi услугата е инсталирана и active, а ESP32 в момента
е offline. Production control остава извън обхвата.

## Completed

- Preserved main documentation from PR #4 while resolving conflicts.
- Removed temporary relay firmware and unused relay pin constants as requested.
- Corrected serial overflow and bounded input processing; checked source NVS writes.
- Disabled legacy direct ESP32 MQTT; synchronized the affected EN/BG data paths.
- Added register-range boundary validation and TCP/serial regression tests.
- Recorded findings and operational limits in docs/PR3_TECHNICAL_REVIEW.md.
- Added a TLS-only, outbound-only MQTT publisher for Edge health and normalized
  node telemetry. It has no MQTT command subscription.
- Added a local Modbus TCP simulator test for continuous node polling and
  deterministic MQTT payload tests.

Запазени са документите от PR #4, премахнат е relay тестът, поправени са
серийният вход и NVS потвърждението, изключен е директният ESP32 MQTT.
Добавени са range проверки и TCP/serial тестове; EN/BG документацията е обновена.
Добавен е TLS-only, outbound-only MQTT publisher за Edge health и нормализирана
node telemetry без MQTT command subscription. Добавени са локален Modbus TCP
simulator тест за постоянен polling и детерминистични MQTT payload тестове.
- Added ESP32 OTA firmware endpoint with a provisioned ROCK Pi source check,
  per-node SHA-256 token verifier and firmware digest check. Added the local,
  non-listening `gridex_ota_apply` ROCK Pi client.
- Recorded this as the separate local OTA update service in the root service
  inventory, hardware baseline and external-interface contract.
- The physical pilot completed a ROCK Pi initiated OTA update, ESP32 reboot,
  and Ethernet/Modbus TCP recovery. ROCK Pi service is enabled/active but its
  commissioning lock and every write approval gate remain `0`.
- Pull Request #8 was merged to `main`; its OTA implementation and separate
  service inventory are now part of the main branch.
- Pull Request #9 reconciled the valid evidence from superseded PR #5 and #7
  into `main`; both conflicted original PRs are closed.
- Verified the restored ESP32 stable address answers to direct ROCK Pi Modbus
  TCP reads. An administrator verified the protected endpoint setting and
  restarted the service; the normalized ROCK Pi node slot is online.
- Verified after that restart that the service remains enabled/active, its
  loopback Modbus TCP input-register response is valid and the OTA client
  self-test succeeds. No field-device write or OTA flash was issued.
- Revalidated a real local OTA update on 2026-09-12: ESP32 accepted the
  SHA-256-verified image and recovered to normal boot mode. ROCK Pi returned
  the normalized node slot to online; commissioning remained locked with every
  write approval gate at `0`.
- Deployed and verified the isolated dual-Ethernet pilot: management retained
  the sole default route, the OT interface received a static protected-env
  address, and interface-bound DHCP issued the ESP32's reserved lease.
- Locally provisioned the ESP32 trusted ROCK Pi OT source address. The ROCK Pi
  read-only normalized Modbus slot then reported the node online with a fresh
  heartbeat. No inverter, BESS or node command was sent.
- Pull Request #13 was merged to `main`. Its ROCK Pi local telemetry build was
  installed on the physical pilot with a local rollback copy. The service and
  OT DHCP are active; all control approval gates remain locked. The post-install
  ESP32 reachability check was offline. The protected journal was subsequently
  confirmed to contain records; map-v5 health requires a separate recovery
  validation.

Pull Request #13 е слят към `main`. ROCK Pi local telemetry build-ът му е
инсталиран на физическия пилот с локално rollback копие. Услугата и OT DHCP са
active; всички control approval gate-ове остават заключени. Post-install
проверката за достижимост на ESP32 беше offline. Защитеният journal беше
потвърден, че съдържа записи; map-v5 health изисква отделна проверка след
възстановяване.

Добавени са ESP32 OTA firmware endpoint с provision-нат ROCK Pi source check,
SHA-256 verifier за отделен token и firmware digest проверка. Добавен е
локалният `gridex_ota_apply` client за ROCK Pi без listener.
Отразена е като отделна локална OTA услуга в root service inventory, hardware
baseline и договора за външни интерфейси.
Физическият пилот изпълни OTA update, стартирано от ROCK Pi, рестарт на ESP32
и възстановяване на Ethernet/Modbus TCP. Услугата на ROCK Pi е enabled/active,
но commissioning lock и всички write approval gate-ове остават `0`.
Pull Request #8 е слят към `main`; OTA имплементацията и отделният service
inventory вече са част от main branch.
Pull Request #9 съгласува валидните доказателства от отменените PR #5 и #7 в
`main`; и двата конфликтни оригинални PR-а са затворени.
Потвърдено е, че възстановеният устойчив ESP32 адрес отговаря на директни ROCK
Pi Modbus TCP reads. Администратор провери защитената endpoint настройка и
рестартира услугата; нормализираният ROCK Pi node slot е online.
След този рестарт е потвърдено, че услугата остава enabled/active, loopback
Modbus TCP input-register отговорът е валиден и OTA client self-test е успешен.
Не е изпратен field-device write и не е изпълнен OTA flash.
На 2026-09-12 е повторена реална локална OTA актуализация: ESP32 прие образа
с проверен SHA-256 и се възстанови в нормален boot режим. ROCK Pi върна
нормализирания node slot в online; commissioning остана заключен и всички
write approval gate-ове са `0`.
- Внедрен и проверен е изолираният dual-Ethernet пилот: management запази
  единствения default route, OT интерфейсът получи статичен адрес от защитения
  environment, а DHCP само за този интерфейс издаде резервирания ESP32 lease.
- Локално е provision-нат довереният ROCK Pi OT source адрес на ESP32.
  Read-only нормализираният Modbus слот на ROCK Pi отчете node online с нов
  heartbeat. Не е изпращана команда към инвертор, BESS или нод.

## Remaining

- Restore the ESP32 on the protected OT segment. Verify its protected endpoint
  setting locally, then confirm that the ROCK Pi records an offline-to-online
  journal transition and periodic snapshot without sending a device command.
- Възстанови ESP32 в защитения OT сегмент. Провери локално protected endpoint
  настройката му, после потвърди, че ROCK Pi записва offline-to-online journal
  transition и периодичен snapshot без изпращане на device команда.
- Provision the private MQTT CA/client identity through the backend secret
  store, then confirm health messages at the broker.

Provision-ни private MQTT CA/client identity чрез backend secret store, след
което потвърди health съобщенията.

## Modified files

`base-rockpie/` adds the bounded local NDJSON journal, explicit polling failure
status and map-v5 node-health ingestion. `node-esp32-evb/` adds local logical
identity provisioning, health registers, watchdog status and recovery hooks.
`docs/LOCAL_TELEMETRY_AND_NODE_PROVISIONING.md` is the matching EN/BG contract.
`docs/TELEMETRY_JOURNAL_RECOVERY_V1.md` is the draft Edge export profile; no
exporter or acknowledgement path is implemented.
`base-rockpie/` добавя ограничения local NDJSON журнал, изрично polling failure
състояние и map-v5 node-health ingestion. `node-esp32-evb/` добавя local
logical identity provisioning, health регистри, watchdog статус и recovery
hooks. `docs/LOCAL_TELEMETRY_AND_NODE_PROVISIONING.md` е съответният EN/BG
договор. `docs/TELEMETRY_JOURNAL_RECOVERY_V1.md` е черновата на Edge export
profile-а; exporter или acknowledgement path не са имплементирани.

`base-rockpie/` now includes a protected OT network installer, an
interface-bound dnsmasq renderer and systemd unit. `docs/OT_DHCP.md` contains
the matching EN/BG procedure; project state, handoff and rules reflect the
verified pilot.
`base-rockpie/` вече включва защитен OT network installer, dnsmasq renderer,
ограничен до интерфейс, и systemd unit. `docs/OT_DHCP.md` съдържа съответната
EN/BG процедура; project state, handoff и правилата отразяват проверения пилот.

## Tests

The private-MQTT-disabled ROCK Pi CMake/CTest suite passed 7/7, including the
new local journal and Modbus node-health simulator checks. Both PlatformIO
profiles (`esp32-evb-can` and `esp32-evb-rs485`) built successfully. No
firmware upload, live broker connection or device write was performed.
Private-MQTT-disabled ROCK Pi CMake/CTest пакетът мина 7/7, включително новите
проверки за local journal и Modbus node-health simulator. И двата PlatformIO
профила (`esp32-evb-can` и `esp32-evb-rs485`) се build-наха успешно. Не е
изпълняван firmware upload, live broker връзка или device write.

MQTT-enabled local CMake/CTest: 5/5 passed, including a local node-polling
Modbus TCP simulator and MQTT payload tests. The current private-MQTT-disabled
build also passed 6/6 CTest checks, including the install target.
`git diff --check` and OT shell/renderer validation passed. No live broker,
hardware upload or device write was performed.
The physical pilot also completed a short loopback-only, read-only preflight:
the existing listener started and its normalized node slot reported the
configured ESP32 as online. The process was stopped after the check.
The isolated OT pilot verified both systemd services active, one management
default route, an ESP32 DHCP lease and an online node slot through a read-only
Modbus TCP register read. Shell syntax and renderer validation passed locally.

MQTT-enabled локални CMake/CTest: 5/5 успешни, включително local node-polling
Modbus TCP simulator и MQTT payload тестове. Текущият build с изключен
private-MQTT също мина 6/6 CTest проверки, включително install target.
`git diff --check` и OT shell/renderer проверките са успешни. Няма тест с live
broker, hardware upload или device write.
Физическият пилот също изпълни кратък loopback-only, read-only preflight:
съществуващият listener стартира и нормализираният му node slot отчете
конфигурирания ESP32 като online. Процесът беше спрян след проверката.
Изолираният OT пилот потвърди active и за двете systemd услуги, един management
default route, ESP32 DHCP lease и online node slot чрез read-only Modbus TCP
register read. Shell syntax и renderer проверките минаха локално.

The standard local sandbox blocks loopback TCP `bind`; the three Modbus server
tests therefore fail there with `Operation not permitted` but passed unchanged
6/6 on the approved host-network execution path.
Стандартният local sandbox блокира loopback TCP `bind`; затова трите Modbus
server теста там падат с `Operation not permitted`, но без промяна минаха 6/6
през одобрения host-network execution path.

## Known issues

UnconfiguredDriver remains deliberate. A private MQTT broker CA/client identity
has not been provisioned. The OTA pilot used a temporary owner-only bench token;
production secret rotation, release signing, router ACL approval and OT soak
tests remain incomplete. The OT pilot deployment lessons are recorded in
`docs/OT_DHCP.md` and `HANDOFF.md`; apply them to every future site.

UnconfiguredDriver е умишлен. Private MQTT broker CA/client identity още не е
provision-нат. OTA pilot-ът използва временен owner-only bench token; production
secret rotation, release signing, router ACL approval и OT soak тестовете
остават незавършени. Уроците от OT pilot deployment-а са записани в
`docs/OT_DHCP.md` и `HANDOFF.md`; прилагай ги за всеки бъдещ site.

## Next action

With the ESP32 restored on the protected OT segment, verify a real local
journal file, its offline-to-online transition and the map-v5 health read.
Do not flash firmware or enable any control path as part of that validation.

След като ESP32 бъде възстановен в защитения OT сегмент, потвърди реален local
journal файл, offline-to-online transition и map-v5 health прочит. Като част от
тази проверка не flash-вай firmware и не включвай control path.

## Last updated

2026-09-13
