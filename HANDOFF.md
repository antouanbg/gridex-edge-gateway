# Handoff — GrideX Edge Gateway

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## PR reconciliation checkpoint / Проверка на PR — 2026-09-24

PRs #18, #19 and #20 merged after MQTT-required build and 8/8 CTest success on macOS. This is not a new board deployment. Legacy draft #2 conflicts with current MQTT publisher, main, CMake and ESP code; do not merge its old transport implementation over the verified one. Review only still-missing changes separately. Ethernet and commissioning locks were not changed.

PR #18, #19 и #20 са слети след MQTT-required build и 8/8 CTest на macOS. Това не е ново внедряване на платката. Старият draft #2 конфликтува с текущия MQTT publisher, main, CMake и ESP код; без сливане на стария транспорт върху проверения. Следва отделен преглед само на липсващи промени. Ethernet и commissioning locks не са променяни.

## CPU telemetry verified live / CPU телеметрията е потвърдена — 2026-09-24

This supersedes the older five-metric/CPU-pending status below. The operator
ran the pinned SHA-256-verified CPU opt-in script on the physical ROCK Pi. It
reported `ROCK_CPU_TEMPERATURE_ACTIVE` for
`/sys/class/thermal/thermal_zone0/temp`, with private rollback at
`/var/backups/gridex-cpu-temperature.nfQasW`. Independent backend checks found
fresh CPU datapoints in OpenRemote TimescaleDB; the OpenRemote datapoint API
returned 21 readings in the last hour with a latest 52.083 °C at the check.
All six system metrics now traverse ROCK → MQTT → backend → OpenRemote. This
does not yet establish owner-browser Devices acceptance or long-term stability.
Commissioning/control locks and battery MODBUS writes were not changed.

Това заменя по-стария статус за пет показателя/чакаща CPU температура.
Операторът изпълни проверения по SHA-256 скрипт на физическия ROCK Pi. Той
върна `ROCK_CPU_TEMPERATURE_ACTIVE` за
`/sys/class/thermal/thermal_zone0/temp` с частен rollback в
`/var/backups/gridex-cpu-temperature.nfQasW`. Независима backend проверка
намери пресни CPU datapoints в OpenRemote TimescaleDB; OpenRemote API върна
21 измервания за последния час с последна стойност 52.083 °C при проверката.
Всичките шест системни показателя вече минават по пътя ROCK → MQTT → backend
→ OpenRemote. Това не доказва още реалния owner екран „Устройства“ или дълга
стабилност. Без промяна на commissioning/control locks и без MODBUS записи
към батерията.

## ROCK pilot active; five metrics stored / ROCK пилотът е активен — 2026-09-24

This entry supersedes the older "service stopped / zero outbox" status below.
The corrected build passed a physical 55-second crash test, then the permanent
activation reported `ROCK_SYSTEM_TELEMETRY_ACTIVE`, one MQTT connection and a
local publish. Backend receipt and OpenRemote Timescale datapoints were later
independently verified: uptime, load1, memory available, data free and journal
size each increased from 15 to 24 records during checks. The backend had to
repair five missing restricted-writer Asset links (403 retries); see backend
HANDOFF for its validated backup. CPU temperature is still missing; inspect
ROCK sensor availability and its opt-in configuration. Long-running stability,
external Devices display and cross-owner denial remain open. No evidence here
authorizes battery MODBUS writes or changes to commissioning locks.

Този запис заменя по-стария статус „услугата е спряна / outbox е празен“.
Поправеният build издържа 55-секунден физически тест, а постоянното включване
върна `ROCK_SYSTEM_TELEMETRY_ACTIVE`, MQTT връзка и публикуване. Backend приемът
и Timescale записите бяха независимо потвърдени: uptime, load1, свободна RAM,
място и journal size нараснаха от 15 до 24 записа за всеки. В backend бяха
поправени пет липсващи връзки към ограничения writer. CPU температура липсва;
провери сензора и настройката на ROCK. Остават дълга стабилност, външен екран
„Устройства“ и отказ за чужд собственик. Това не разрешава MODBUS записи към
батерия или промяна на commissioning locks.

CPU temperature is opt-in (`GRIDEX_CPU_TEMPERATURE_ENABLED=0` by default),
which explains the five rather than six published samples. The new
`base-rockpie/install/enable-cpu-temperature.sh` is a one-file pilot operation:
it validates a readable Linux thermal sensor, preserves commissioning locks,
backs up the existing env, restarts only the ROCK service, checks stable PID and
six published samples, and restores the env on failure. It is installed in
future image payloads. It has not been executed on the physical ROCK because
noninteractive SSH was denied; a local operator must run it once. A six-sample
local publish still requires separate backend/Timescale verification.

CPU температурата е изключена по подразбиране (`GRIDEX_CPU_TEMPERATURE_ENABLED=0`),
затова се публикуват пет, а не шест проби. Новият еднофайлов скрипт
`base-rockpie/install/enable-cpu-temperature.sh` проверява четим Linux thermal
сензор, пази commissioning locks, архивира env, рестартира само ROCK услугата,
проверява стабилен PID и шест публикувани проби и връща env при отказ.
Добавен е към бъдещия image payload. Не е изпълнен на физическия ROCK, понеже
автоматичният SSH вход е отказан; оператор трябва да го стартира веднъж.
Локално публикуване на шест проби още изисква отделна backend/Timescale проверка.

## ROCK system telemetry crash recovery — 2026-09-23

The physical attempt built commit `927d73a` on ROCK Pi, but the service failed
the 45-second stability check. The installer restored the preceding binary and
configuration; that binary also crashed, so it stopped the service. The pilot
ROCK Pi service remains STOPPED. No live ROCK heartbeat, system telemetry, or
stored system datapoint has been verified. Control approval gates remain zero.

The controlled `gdb` run of the **rolled-back old binary** captured SIGSEGV in
`libmosquitto.so.1` during `mosquitto_publish_v5()`, called from
`MqttHealthPublisher::publishNodeTelemetry()`. A separate `mosquitto loop`
thread was active, while the node polling and Modbus server threads were not
crashing. This supports an MQTT client concurrency problem in the old binary;
it does **not** identify why the newer candidate failed its stability check.
The earlier "MQTT offline" explanation was incorrect: connection result was 0.

Next controlled action: run the updated
`base-rockpie/install/capture-rock-crash-stack.sh` once. It builds the current
candidate from `feat/rock-temperature` with symbols into a separate debug binary,
runs that candidate under the existing service identity/environment using a
runtime-only `Restart=no` override, captures its stack, removes the override
and temporary binary, and leaves the service stopped. It does not replace the
installed binary/configuration or print the protected env. Do **not** rerun
`activate-system-telemetry.sh` until the candidate is diagnosed and a fix
passes physical validation. Claim live data only after fresh MQTT receipt and
an independently verified OpenRemote/Timescale datapoint.

First candidate diagnostic attempt (`de9f42d`) did not execute its binary:
systemd/gdb reported `Permission denied` for the temporary `/run` path. This
was a diagnostic staging-path failure, not evidence of a new application crash.
The script now stages the separate `gridex_rockpie_debug` executable next to
the known executable service binary in `/usr/local/bin`, refuses an existing
file at that exact path, and removes only its own file afterward.

The next controlled run (`fc1c012`) reproduced SIGSEGV in the newer candidate,
now at `mosquitto_loop()` called from `MqttHealthPublisher::pump()` on the main
thread. Code review found two concrete defects: `GRIDEX_WITH_MOSQUITTO` was a
PRIVATE compile definition while the public header conditionally changed the
class layout, so the service and transport library disagreed on the object's
size; and `mosquitto_connect_async()` was combined with manual
`mosquitto_loop()`, contrary to the Mosquitto API contract. The candidate now
keeps layout independent of the flag, exports the flag to consumers, and pairs
async connect with `mosquitto_loop_start()`. An ABI regression test compares
class size compiled with/without the flag. macOS build and all eight tests
pass (loopback tests required local-port sandbox approval). The same eight tests
pass in an isolated ARM64 Linux container with libmosquitto. Physical validation
of this new candidate is PENDING; service remains stopped, and the old binary
has not been replaced. Run the one-shot capture again before activation.
The later pasted trace still identifies commit `fc1c012` and `pump()`: it is
the previous diagnostic result, not evidence against the fix. GitHub branch
HEAD was verified as `372b375`. The diagnostic script now refuses source that
still calls `pump()` or lacks `mosquitto_loop_start()`.

Physical one-shot of corrected `cf834fc` PASSED: 55 seconds without a crash,
MQTT CONNACK success, and two successful local QoS1 publish calls with five
system samples each. `writes_enabled=false` and `commissioning_locked` stayed
intact. The diagnostic deliberately stopped the service afterward; the
installed binary was not replaced. Broker logs confirm the pilot MQTT client
connected. Backend history worker has six configured system bindings and an
active subscription, but the read-only outbox query returned ZERO rows as of
2026-09-23 20:59 UTC. Local `publish=true` is not a broker acknowledgement or
stored datapoint, so backend delivery/history is NOT verified. The activation
script now rejects obsolete source and requires stable PID, successful MQTT
connect log and a system telemetry publish log within 45 seconds; its rollback
remains in place. After activation, independently check the broker and outbox
before calling telemetry live.
The active broker ACL file was modified at 2026-09-23 15:39 UTC, while the
broker process had been running since 2026-09-15. No ACL reload was seen in
the intervening logs. A scoped SIGHUP was sent to the broker on 2026-09-23
to reload that file without restarting the container; it remained healthy.
This is a likely explanation for the zero outbox rows, NOT yet proven. The
next physical ROCK activation and subsequent broker/outbox checks will test it.

Физическият опит компилира commit `927d73a` на ROCK Pi, но услугата не издържа
45-секундната проверка. Инсталаторът върна предишния binary/config; и той
падна, затова услугата остана СПРЯНА. Няма потвърден live ROCK heartbeat,
системна телеметрия или записана system datapoint стойност. Control approval
gate-овете остават нула.

Контролираният `gdb` тест на **върнатия стар binary** улови SIGSEGV в
`libmosquitto.so.1` при `mosquitto_publish_v5()`, извикан от
`MqttHealthPublisher::publishNodeTelemetry()`. Отделна `mosquitto loop` нишка
работеше; node polling и Modbus server нишките не бяха мястото на crash-а.
Това подкрепя проблем с едновременен достъп до MQTT клиента в стария binary,
но **не** доказва причината за неуспеха на новия кандидат. Старото обяснение
„MQTT offline“ беше погрешно: резултатът от свързването беше 0.

Следва еднократно изпълнение на обновения
`base-rockpie/install/capture-rock-crash-stack.sh`. Той компилира текущия
кандидат от `feat/rock-temperature` със symbols като отделен debug binary,
пуска го с текущите service identity/env и runtime-only override с
`Restart=no`, събира stack, почиства временните файлове и оставя услугата
спряна. Не подменя инсталирания binary/config и не показва защитения env.
**Не** пускай `activate-system-telemetry.sh` пак, преди кандидатът да бъде
диагностициран и поправката да мине физически тест. Live данни се заявяват
едва след ново MQTT съобщение и отделно потвърден OpenRemote/Timescale запис.

Първият диагностичен опит на кандидата (`de9f42d`) не изпълни binary:
systemd/gdb върна `Permission denied` за временния път в `/run`. Това е отказ
на диагностичния staging път, не нов доказан application crash. Скриптът вече
поставя отделния `gridex_rockpie_debug` до изпълнимия service binary в
`/usr/local/bin`, отказва съществуващ файл на този точен път и после премахва
само своя временен файл.

Следващият контролиран опит (`fc1c012`) възпроизведе SIGSEGV и в новия
кандидат — този път в `mosquitto_loop()`, извикан от
`MqttHealthPublisher::pump()` в основната нишка. Прегледът откри два конкретни
дефекта: `GRIDEX_WITH_MOSQUITTO` беше PRIVATE compile flag, а публичният header
променяше размера на класа според него, така че service и transport библиотеката
не бяха съгласни за размера на обекта; `mosquitto_connect_async()` беше съчетан
с ръчен `mosquitto_loop()`, което противоречи на Mosquitto API договора.
Кандидатът вече има независим от flag-а class layout, изнася flag-а към
потребителите и съчетава async connect с `mosquitto_loop_start()`. ABI
регресионен тест сравнява размера при компилация със/без flag. macOS build и
осемте теста минават (loopback тестовете изискваха разрешен локален порт).
Същите осем теста минават и в изолиран ARM64 Linux контейнер с libmosquitto.
Физическата проверка ПРЕДСТОИ; услугата остава спряна и старият binary не е
подменян. Първо повтори еднократния диагностичен тест, не активацията.
По-късно изпратеният stack отново сочи commit `fc1c012` и `pump()` — това е
предишният резултат, не доказателство срещу поправката. GitHub branch HEAD е
проверен като `372b375`. Диагностичният скрипт вече отказва стар source с
`pump()` или без `mosquitto_loop_start()`.

Физическият еднократен тест на поправения `cf834fc` МИНА: 55 секунди без crash,
успешно MQTT свързване и две успешни локални QoS1 publish извиквания с по пет
системни измервания. `writes_enabled=false` и `commissioning_locked` останаха.
След теста диагностиката умишлено спря услугата; инсталираният binary не е
подменен. Broker log потвърждава връзката на пилотния MQTT клиент. Backend
history worker има шест конфигурирани system bindings и активен абонамент, но
read-only проверката на outbox върна НУЛА реда към 2026-09-23 20:59 UTC.
Локалното `publish=true` не е broker acknowledgement или записан datapoint,
затова backend доставката/историята НЕ са потвърдени. Скриптът за активация
вече отказва стар source и изисква стабилен PID, успешен MQTT connect log и
system telemetry publish log в 45 секунди; rollback е запазен. След активация
broker и outbox се проверяват отделно преди да се обяви live телеметрия.
Активният broker ACL файл е променен на 2026-09-23 15:39 UTC, а broker
процесът работеше още от 2026-09-15. В междинните логове няма ACL reload.
На 2026-09-23 е изпратен ограничен SIGHUP за презареждане на файла без
рестарт на контейнера; broker остана healthy. Това вероятно обяснява празния
outbox, но още НЕ е доказано. Следващата физическа активация на ROCK и
последващите broker/outbox проверки ще го проверят.

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
