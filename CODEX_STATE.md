# CODEX_STATE.md

Repository / GitHub: antouanbg/gridex-edge-gateway

## Current task

Technical review, corrections and user-authorized merge of PR #3.
Технически преглед, корекции и разрешено от потребителя сливане на PR #3.

## Completed

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

- Complete the approved GitHub merge; verify its state and main commit.
- Commissioning and MQTT forwarding remain tracked in HANDOFF.md.

Завърши разрешеното сливане в GitHub и провери main. Commissioning и MQTT
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

After merging PR #3, read AGENTS.md and HANDOFF.md and select the next
commissioning/driver milestone. Keep device writes locked.

След сливане на PR #3 прочети AGENTS.md и HANDOFF.md и избери следващия
commissioning/driver етап. Запази блокираните записи към устройства.

## Last updated

2026-09-12
