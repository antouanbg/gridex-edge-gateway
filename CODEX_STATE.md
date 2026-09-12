# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Document the ordered local commissioning sequence that does not require a
running backend.
Документиране на последователния локален commissioning, който не изисква
работещ backend.

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
- Reconciled `HANDOFF.md` with the local-only work list: deployment, polling
  verification, OT isolation, ESP provisioning, telemetry validation, local
  retention, commissioning view, recovery tests and read-only soak testing.

Запазени са документите от PR #4, премахнат е relay тестът, поправени са
серийният вход и NVS потвърждението, изключен е директният ESP32 MQTT.
Добавени са range проверки и TCP/serial тестове; EN/BG документацията е обновена.
Добавен е TLS-only, outbound-only MQTT publisher за Edge health и нормализирана
node telemetry без MQTT command subscription. Добавени са локален Modbus TCP
simulator тест за постоянен polling и детерминистични MQTT payload тестове.
`HANDOFF.md` е синхронизиран с local-only списъка: deployment, проверка на
polling, OT изолация, ESP provisioning, telemetry validation, local retention,
commissioning view, recovery тестове и read-only soak.

## Remaining

- Review and merge the documentation PR, then perform step 1 of the ordered
  local commissioning sequence with all write gates locked.

Прегледай и merge-ни документационния PR, след което изпълни стъпка 1 от
последователния local commissioning със заключени write gate-ове.

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

Review the ordered sequence in `HANDOFF.md`, then deploy the read-only service
from step 1 with node polling enabled.

Прегледай последователността в `HANDOFF.md`, след което внедри read-only
услугата от стъпка 1 с включен node polling.

## Last updated

2026-09-12
