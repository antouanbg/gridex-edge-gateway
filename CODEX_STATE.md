# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Document and apply the per-site node network provisioning convention, then
verify the protected ROCK Pi endpoint setting with a local administrator.
Документиране и прилагане на per-site convention за мрежово provision-ване на
нод, след което проверка на защитената ROCK Pi endpoint настройка с локален
администратор.

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
  TCP reads, while the protected active ROCK Pi polling configuration still
  requires administrator inspection because its normalized node slot is offline.

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
Pi Modbus TCP reads, докато защитената активна ROCK Pi polling конфигурация още
изисква проверка от администратор, защото нормализираният node slot е offline.

## Remaining

- Provision the private MQTT CA/client identity through the backend secret
  store, then confirm health messages at the broker.
- With a local administrator, verify/update `GRIDEX_NODE_ENDPOINTS`, restart
  the locked read-only service and confirm the node slot returns online.

Provision-ни private MQTT CA/client identity чрез backend secret store, след
което потвърди health съобщенията.
С локален администратор провери/обнови `GRIDEX_NODE_ENDPOINTS`, рестартирай
заключената read-only услуга и потвърди, че node slot-ът се връща online.

## Modified files

`base-rockpie/` adds `gridex_ota_apply`; `node-esp32-evb/` adds the protected
OTA endpoint; `docs/ESP32_OTA.md` defines the EN/BG procedure. Existing pilot,
handoff and project rules reflect the verified recovery.
`base-rockpie/` добавя `gridex_ota_apply`; `node-esp32-evb/` добавя защитен
OTA endpoint; `docs/ESP32_OTA.md` описва EN/BG процедурата. Съществуващите
pilot, handoff и project правила отразяват потвърденото възстановяване.

## Tests

MQTT-enabled local CMake/CTest: 5/5 passed, including a local node-polling
Modbus TCP simulator and MQTT payload tests. `git diff --check` passed.
No live broker, hardware upload or device write was performed.
The physical pilot also completed a short loopback-only, read-only preflight:
the existing listener started and its normalized node slot reported the
configured ESP32 as online. The process was stopped after the check.

MQTT-enabled локални CMake/CTest: 5/5 успешни, включително local node-polling
Modbus TCP simulator и MQTT payload тестове. `git diff --check` е успешен.
Няма тест с live broker, hardware upload или device write.
Физическият пилот също изпълни кратък loopback-only, read-only preflight:
съществуващият listener стартира и нормализираният му node slot отчете
конфигурирания ESP32 като online. Процесът беше спрян след проверката.

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

Use the local administrator session to inspect/update the protected
`GRIDEX_NODE_ENDPOINTS` entry, restart the service and re-read slot `0x0100`.

Използвай локална администраторска сесия, за да провериш/обновиш защитената
`GRIDEX_NODE_ENDPOINTS` стойност, рестартираш услугата и прочетеш отново slot
`0x0100`.

## Last updated

2026-09-12
