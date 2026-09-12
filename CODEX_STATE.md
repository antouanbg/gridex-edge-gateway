# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Implement ROCK Pi continuous node polling, northbound Modbus listener and
private MQTT health/telemetry.
Имплементиране на постоянен node polling, northbound Modbus listener и private
MQTT health/telemetry на ROCK Pi.

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

## Remaining

- Build/deploy the latest ROCK Pi service with the explicit bench-node endpoint
  and keep all write gates locked.
- Provision the private MQTT CA/client identity through the backend secret
  store, then confirm health messages at the broker.

Изгради/внедри последната ROCK Pi услуга с изричния bench-node endpoint и
запази всички write gate-ове заключени. Provision-ни private MQTT CA/client
identity чрез backend secret store, след което потвърди health съобщенията.

## Modified files

`base-rockpie/` adds the private MQTT publisher, configuration and focused
tests; docs/ROCKPI_PRIVATE_MQTT_HEALTH.md defines the contract.
`base-rockpie/` добавя private MQTT publisher, конфигурация и фокусирани
тестове; docs/ROCKPI_PRIVATE_MQTT_HEALTH.md описва договора.

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
has not been provisioned, and the physical pilot has not received this binary.
OT commissioning and soak tests remain incomplete.

UnconfiguredDriver е умишлен. Private MQTT broker CA/client identity още не е
provision-нат, а физическият пилот не е получил този binary. OT commissioning
и soak тестовете остават незавършени.

## Next action

Deploy the built service read-only to the pilot with node polling enabled;
after broker provisioning, verify the two documented MQTT topic families.

Внедри built услугата read-only на пилота с включен node polling; след broker
provisioning потвърди двете описани MQTT topic семейства.

## Last updated

2026-09-12
