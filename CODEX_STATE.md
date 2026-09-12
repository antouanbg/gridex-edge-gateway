# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

No active implementation task. The isolated OT networking change is ready for
review as a Pull Request; production control remains out of scope.
Няма активна задача по имплементация. Промяната за изолираната OT мрежа е
готова за review като Pull Request; production control остава извън обхвата.

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

- Provision the private MQTT CA/client identity through the backend secret
  store, then confirm health messages at the broker.

Provision-ни private MQTT CA/client identity чрез backend secret store, след
което потвърди health съобщенията.

## Modified files

`base-rockpie/` now includes a protected OT network installer, an
interface-bound dnsmasq renderer and systemd unit. `docs/OT_DHCP.md` contains
the matching EN/BG procedure; project state, handoff and rules reflect the
verified pilot.
`base-rockpie/` вече включва защитен OT network installer, dnsmasq renderer,
ограничен до интерфейс, и systemd unit. `docs/OT_DHCP.md` съдържа съответната
EN/BG процедура; project state, handoff и правилата отразяват проверения пилот.

## Tests

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

## Known issues

UnconfiguredDriver remains deliberate. A private MQTT broker CA/client identity
has not been provisioned. The OTA pilot used a temporary owner-only bench token;
production secret rotation, release signing, router ACL approval and OT soak
tests remain incomplete.

UnconfiguredDriver е умишлен. Private MQTT broker CA/client identity още не е
provision-нат. OTA pilot-ът използва временен owner-only bench token; production
secret rotation, release signing, router ACL approval и OT soak тестовете
остават незавършени.

## Next action

Before the next task, read `AGENTS.md`, `CODEX_STATE.md` and `HANDOFF.md`,
then inspect the actual repository and device state. The next implementation
priority is private MQTT identity provisioning, a supervised OT soak test, or
a separately authorized read-only device-driver task.

Преди следващата задача прочети `AGENTS.md`, `CODEX_STATE.md` и
`HANDOFF.md`, след което провери действителното състояние на repository-то и
устройствата. Следващият приоритет за имплементация е private MQTT identity
provisioning, наблюдаван OT soak test или отделно оторизирана read-only задача
за device driver.

## Last updated

2026-09-12
