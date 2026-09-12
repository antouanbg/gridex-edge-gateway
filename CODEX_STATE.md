# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

No active implementation task. The local, ROCK Pi mediated ESP32 OTA change is
complete and awaiting owner review in Pull Request #8.
Няма активна имплементационна задача. Локалната ESP32 OTA промяна през ROCK Pi
е завършена и очаква преглед от собственика в Pull Request #8.

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
- The physical pilot completed a ROCK Pi initiated OTA update, ESP32 reboot,
  and Ethernet/Modbus TCP recovery. ROCK Pi service is enabled/active but its
  commissioning lock and every write approval gate remain `0`.

Добавени са ESP32 OTA firmware endpoint с provision-нат ROCK Pi source check,
SHA-256 verifier за отделен token и firmware digest проверка. Добавен е
локалният `gridex_ota_apply` client за ROCK Pi без listener.
Физическият пилот изпълни OTA update, стартирано от ROCK Pi, рестарт на ESP32
и възстановяване на Ethernet/Modbus TCP. Услугата на ROCK Pi е enabled/active,
но commissioning lock и всички write approval gate-ове остават `0`.

## Remaining

- Review the OTA Pull Request. Do not merge/deploy to production until the
  Site Router ACL, release-signing owner and per-node secret rotation process
  are approved.
- Provision the private MQTT CA/client identity through the backend secret
  store, then confirm health messages at the broker.

Прегледай OTA Pull Request-а. Не merge-вай/внедрявай за production преди да
са одобрени Site Router ACL, собственикът на release signing и процесът за
rotation на secret за отделен нод. Provision-ни private MQTT CA/client identity
чрез backend secret store, след което потвърди health съобщенията.

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

Owner review of Pull Request #8, then separately approve Site Router ACL,
release-signing ownership and per-node secret rotation before production use.

Преглед от собственика на Pull Request #8, след което отделно се одобряват
Site Router ACL, собственикът на release signing и rotation на secret за
отделен нод преди production употреба.

## Last updated

2026-09-12
