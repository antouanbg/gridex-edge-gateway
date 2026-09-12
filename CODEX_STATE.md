# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Completed read-only live inspection of ROCK Pi and ESP32.
Завършена read-only проверка на включените ROCK Pi и ESP32.

## Completed

- PR #3 merged to main as 149f76e; latest binaries have not been deployed.
- Read-only SSH/serial inspection and three successful ESP32 Modbus reads
  from ROCK Pi are recorded in docs/LIVE_HARDWARE_STATUS.md.

PR #3 е слят в main като 149f76e; последните бинарни файлове не са внедрени.
SSH/serial проверката и трите успешни ESP32 Modbus четения от ROCK Pi са
описани в docs/LIVE_HARDWARE_STATUS.md.

- Preserved main documentation from PR #4 while resolving conflicts.
- Removed temporary relay firmware and unused relay pin constants as requested.
- Corrected serial overflow and bounded input processing; checked source NVS writes.
- Disabled legacy direct ESP32 MQTT; synchronized the affected EN/BG data paths.
- Added register-range boundary validation and TCP/serial regression tests.
- Recorded findings and operational limits in docs/PR3_TECHNICAL_REVIEW.md.

Запазени са документите от PR #4, премахнат е relay тестът, поправени са
серийният вход и NVS потвърждението, изключен е директният ESP32 MQTT.
Добавени са range проверки и TCP/serial тестове; EN/BG документацията е обновена.

## Remaining

- Review the live-status documentation PR.
- Commissioning and MQTT forwarding remain tracked in HANDOFF.md.

Прегледай документационния PR за текущия статус. Commissioning и MQTT
препращането остават описани в HANDOFF.md.

## Modified files

See the PR #3 diff and docs/PR3_TECHNICAL_REVIEW.md for the reviewed scope.
Виж diff на PR #3 и docs/PR3_TECHNICAL_REVIEW.md за прегледания обхват.

## Tests

Fresh local Debug CMake/CTest: base/core 3/3, node 1/1 passed.
PlatformIO CAN and RS485 builds: both passed. Local TCP simulator only.
git diff --check passed. No live hardware tests or uploads in this review.

Нови локални Debug CMake/CTest: base/core 3/3, node 1/1 успешни.
PlatformIO CAN и RS485: успешни. TCP тестът ползва само локален симулатор.
git diff --check е успешен. Няма тестове на жив хардуер или upload в този review.

## Known issues

UnconfiguredDriver remains deliberate. Before implementing a live driver,
validate vendor maps, wiring and command lifecycle. ROCK Pi MQTT forwarding,
OT network commissioning and soak tests remain incomplete. Earlier physical
pilot evidence does not validate the latest reviewed binaries.

UnconfiguredDriver е умишлен. Преди реален driver валидирай картите,
окабеляването и command lifecycle. ROCK Pi MQTT, OT commissioning и soak
тестовете остават незавършени. Старите физически тестове не валидират
последните прегледани бинарни файлове.

## Next action

Read docs/LIVE_HARDWARE_STATUS.md and HANDOFF.md; plan the latest binary
deployment and exact read-only inverter driver. Keep device writes locked.

Прочети docs/LIVE_HARDWARE_STATUS.md и HANDOFF.md; планирай внедряването на
новите бинарни файлове и точния read-only driver. Запази блокираните записи.

## Last updated

2026-09-12
