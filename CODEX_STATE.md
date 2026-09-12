# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Deploy the current ROCK Pi service in a locked, read-only node-polling mode.
Внедряване на текущата ROCK Pi услуга в заключен read-only режим за node polling.

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
- Staged the exact merged source in a separate ROCK Pi release directory,
  built it natively with MQTT disabled (no broker exists), and passed all five
  CTest tests.
- Installed the service with a non-production local configuration: listener on
  loopback, ESP32 node polling enabled and every BESS write approval locked.
  Systemd enablement, active status and the online normalized node slot were
  verified.

Запазени са документите от PR #4, премахнат е relay тестът, поправени са
серийният вход и NVS потвърждението, изключен е директният ESP32 MQTT.
Добавени са range проверки и TCP/serial тестове; EN/BG документацията е обновена.
Добавен е TLS-only, outbound-only MQTT publisher за Edge health и нормализирана
node telemetry без MQTT command subscription. Добавени са локален Modbus TCP
simulator тест за постоянен polling и детерминистични MQTT payload тестове.
`HANDOFF.md` е синхронизиран с local-only списъка: deployment, проверка на
polling, OT изолация, ESP provisioning, telemetry validation, local retention,
commissioning view, recovery тестове и read-only soak.
Точният merge-нат source е поставен в отделна release директория на ROCK Pi,
изграден native с изключен MQTT (няма broker) и петте CTest теста са успешни.
Услугата е инсталирана с non-production local конфигурация: listener само на
loopback, включен ESP32 node polling и заключени всички BESS write разрешения.
Потвърдени са systemd enablement, active status и online нормализиран node slot.

## Remaining

- Execute step 2 of the ordered local commissioning sequence: controlled
  disconnect/reconnect of the ESP32 and evidence that its status changes to
  `online=false` without disrupting the service.

Изпълни стъпка 2 от последователния local commissioning: контролирано
изключване/свързване на ESP32 и доказателство, че статусът му става
`online=false`, без да се прекъсва услугата.

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

Perform the controlled ESP32 disconnect/reconnect test from step 2, then record
the observed node-offline transition in the pilot evidence.

Изпълни контролирания ESP32 disconnect/reconnect тест от стъпка 2, след което
запиши наблюдавания node-offline преход в pilot evidence.

## Last updated

2026-09-12
