# CODEX_STATE.md

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
