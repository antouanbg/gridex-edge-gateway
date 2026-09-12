# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Verify controlled recovery of the locked, read-only ROCK Pi and ESP32 pilot.
Потвърждаване на контролирано възстановяване на заключения read-only ROCK Pi и ESP32 пилот.

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

## Remaining

- Complete the remaining step-2 physical Ethernet disconnect/reconnect test
  and record `online=false` followed by recovery, without disrupting the
  service or enabling any device command.

Завърши оставащия физически Ethernet disconnect/reconnect тест от стъпка 2 и
запиши `online=false`, последвано от recovery, без прекъсване на услугата или
разрешаване на device команда.

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
reset, ROCK Pi reboot, systemd automatic recovery and the normalized node slot
returning online all passed. No device write was performed.

MQTT-enabled локални CMake/CTest: 5/5 успешни, включително local node-polling
Modbus TCP simulator и MQTT payload тестове. `git diff --check` е успешен.
Няма тест с live broker, hardware upload или device write.
Физическият пилот изпълни loopback-only, read-only recovery проверки: ESP32
reset, ROCK Pi reboot, автоматично възстановяване от systemd и връщане на
нормализирания node slot към online са успешни. Не е извършвана device write
операция.

## Known issues

UnconfiguredDriver remains deliberate. A private MQTT broker CA/client identity
has not been provisioned. OT commissioning, a physical Ethernet disconnect test
and soak tests remain incomplete.

UnconfiguredDriver е умишлен. Private MQTT broker CA/client identity още не е
provision-нат. OT commissioning, физически Ethernet disconnect тест и soak
тестовете остават незавършени.

## Next action

Perform the remaining physical ESP32 Ethernet disconnect/reconnect test from
step 2, then record the observed node-offline and recovery transitions.

Изпълни оставащия физически ESP32 Ethernet disconnect/reconnect тест от
стъпка 2, след което запиши наблюдаваните node-offline и recovery преходи.

## Last updated

2026-09-12
