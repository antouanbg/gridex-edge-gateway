# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

No active task.
Няма активна задача.

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
- Completed an ESP32 reset recovery check: the configured node returned to
  `online=1` without enabling any device write path.
- Completed a controlled ROCK Pi reboot recovery check: systemd automatically
  restarted the service, restored the loopback-only listener and reported the
  configured ESP32 node online.
- Completed the physical ESP32 Ethernet disconnect/reconnect test: the
  normalized node state changed offline to online while the ROCK Pi service
  and loopback-only listener remained active.

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
Изпълнена е recovery проверка след reset на ESP32: конфигурираният нод се върна
към `online=1`, без да се разрешава device write път.
Изпълнена е контролирана recovery проверка след ROCK Pi reboot: systemd
автоматично стартира услугата, възстанови listener-а само на loopback и отчете
конфигурирания ESP32 нод като online.
Изпълнен е физическият ESP32 Ethernet disconnect/reconnect тест:
нормализираният node state премина offline → online, докато ROCK Pi услугата
и listener-ът само на loopback останаха active.

## Remaining

- Multi-node isolation test: verify that a second node can transition offline
  without blocking the other configured node slots. This needs a second node.

Multi-node изолационен тест: провери, че втори нод може да премине offline,
без да блокира другите конфигурирани node slot-ове. Нужен е втори нод.

## Modified files

`base-rockpie/` adds the private MQTT publisher, configuration and focused
tests; docs/ROCKPI_PRIVATE_MQTT_HEALTH.md defines the contract.
`base-rockpie/` добавя private MQTT publisher, конфигурация и фокусирани
тестове; docs/ROCKPI_PRIVATE_MQTT_HEALTH.md описва договора.

## Tests

MQTT-enabled local CMake/CTest: 5/5 passed, including a local node-polling
Modbus TCP simulator and MQTT payload tests. `git diff --check` passed.
No live broker, hardware upload or device write was performed.
The physical pilot completed loopback-only, read-only recovery checks: ESP32
reset, Ethernet disconnect/reconnect, ROCK Pi reboot, systemd automatic
recovery and the normalized node slot returning online all passed. No device
write was performed.

MQTT-enabled локални CMake/CTest: 5/5 успешни, включително local node-polling
Modbus TCP simulator и MQTT payload тестове. `git diff --check` е успешен.
Няма тест с live broker, hardware upload или device write.
Физическият пилот изпълни loopback-only, read-only recovery проверки: ESP32
reset, Ethernet disconnect/reconnect, ROCK Pi reboot, автоматично
възстановяване от systemd и връщане на нормализирания node slot към online са
успешни. Не е извършвана device write операция.

## Known issues

UnconfiguredDriver remains deliberate. A private MQTT broker CA/client identity
has not been provisioned. OT commissioning, multi-node isolation and soak tests
remain incomplete.

UnconfiguredDriver е умишлен. Private MQTT broker CA/client identity още не е
provision-нат. OT commissioning, multi-node изолация и soak тестовете остават
незавършени.

## Next action

When a second ESP32 node is available, perform the multi-node isolation test;
otherwise proceed only with the documented OT-network commissioning sequence.

Когато има втори ESP32 нод, изпълни multi-node изолационния тест; иначе
продължи единствено с документираната последователност за OT commissioning.

## Last updated

2026-09-12
