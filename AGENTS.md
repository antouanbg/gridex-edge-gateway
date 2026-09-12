# GrideX Edge Gateway — Engineering Rules

## Architecture and safety

- The Site Router, not ROCK Pi or ESP32, terminates the WireGuard tunnel.
- ESP32 nodes are on the OT network. ROCK Pi E polls them through Modbus TCP
  and is the sole bridge to private MQTT through the Site Router VPN.
- Do not add WireGuard, public MQTT credentials or direct cloud commands to an
  ESP32 node.
- Vendor maps, signs, scaling and serial/CAN details belong to a device driver.
  Commands must be validated, clamped, time-limited, logged and fail safe.

## ROCK Pi E operating-system baseline

The current **pilot** operating-system candidate for the Radxa ROCK Pi E is:

- Armbian 26.8.1 Minimal CLI, Debian 13 (Trixie), ARM64;
- `Armbian_26.8.1_Rockpi-e_trixie_current_6.18.43_minimal.img.xz`;
- SHA256 `85def0ac69ed7f5d1c1a43d6d0830db0ff92ca698b4bb5c62880ee8cad384813`;
- the Armbian `current` / Linux 6.18.43 Rockchip kernel line.
- no desktop environment and no Docker workload on the ROCK Pi E.

This choice is made because the GrideX base service needs CMake 3.20+, a
C++20-capable compiler and current MQTT packages. Armbian lists this ROCK Pi E
minimal image as tested and stable. Do not use the vendor Debian Buster image
as a GrideX production operating system: it is useful only as a
hardware-reference image for vendor dual-Ethernet validation and its userspace
is end of life.

The pilot candidate is **not** a production approval until it passes the
ROCK Pi E acceptance procedure on the physical board. Before a site deployment:

1. record the exact image filename, release, kernel version and SHA256;
2. verify both Ethernet interfaces are present and stable simultaneously;
3. assign WAN/management to the Site Router and a separate static OT interface
   without a default gateway;
4. verify boot, 20 controlled reboots, power-loss recovery and serial console;
5. build and test the native C++20 GrideX service on the board;
6. test Modbus TCP to the BESS, Modbus TCP polling to ESP32 nodes, northbound
   Modbus, and private MQTT through the Site Router VPN;
7. run a 24-hour telemetry soak test with writes disabled.

Pin the accepted image and its package/kernel update policy in repository
documentation after these tests. Never enable an untested kernel upgrade on a
commissioned site. The Site Router remains the sole WireGuard endpoint.

### Initial microSD provisioning procedure

Use this procedure only on a new or explicitly approved removable microSD card.
It is destructive: the selected card is erased completely.

1. Download the pinned image and verify its SHA256 before it is written.
2. Identify the physical removable card by model and capacity using a read-only
   disk listing. Show the exact disk identifier and obtain explicit user
   confirmation before erase/write; never infer the target from its name.
3. Use Balena Etcher on macOS: choose the verified `.img.xz` file, select only
   the confirmed removable card, start Flash, and wait for Etcher validation to
   complete. Etcher handles administrator authorisation locally; never request
   or record that password.
4. Eject the card cleanly, insert it in the ROCK Pi E, and power the board from
   a stable 5 V / 2 A or higher supply. USB computer power is acceptable only
   for a short bench test.
5. Connect LAN to the Site Router, discover the DHCP lease, and use SSH or the
   1,500,000 baud UART console for first boot diagnosis. Change initial
   credentials before installing GrideX services; do not commit credentials or
   obtained device addresses.
6. Keep all BESS/PCS writes locked for the first boot. Complete the hardware
   acceptance procedure above before enabling any command path.

## OLIMEX ESP32-EVB — verified local bench knowledge

The supported node board family is OLIMEX ESP32-EVB / ESP32-EVB-EA-IND.

- PlatformIO environment: `board = esp32-evb`, Arduino framework.
- The USB serial console uses 115200 bps. Keep one serial connection open while
  issuing a bench command: opening and closing the port for each command can
  reset the ESP32 and lose the command.
- Upload at `115200` bps on this setup. The higher 921600 bps upload rate was
  observed to corrupt the transfer after the ESP32 bootloader connected.

### OTA rule / Правило за OTA

- ESP32 OTA is local ROCK Pi → ESP32 Ethernet only. The Site Router controls
  any WireGuard access to the ROCK Pi; it must not forward WireGuard or public
  Internet traffic directly to ESP32.
- OTA must remain disabled until local serial provisioning records a single
  ROCK Pi source address and a per-node secret verifier. Store only a hash on
  the ESP32 and never commit a token, staging image, node address or device
  identity.
- The ROCK Pi OTA client has no listener. It must validate firmware SHA-256,
  use an owner-only token file, and be run only by an approved operator.
- After opening a USB serial session, wait for the ESP32 boot message before
  issuing a provisioning command; opening a serial port can reset the board.

- OTA за ESP32 е само по локален Ethernet път ROCK Pi → ESP32. Site Router
  управлява всеки WireGuard достъп до ROCK Pi; той не трябва да препраща
  WireGuard или публичен Интернет трафик директно към ESP32.
- OTA остава изключено, докато local serial provisioning не запише единствен
  source адрес на ROCK Pi и verifier за отделна тайна на нода. На ESP32 се
  записва само hash; не записвай в Git token, staging image, адрес на нод или
  идентификатор на устройство.
- OTA client-ът на ROCK Pi няма listener. Той валидира SHA-256 на firmware-а,
  използва token файл само за собственика и се изпълнява единствено от одобрен
  оператор.
- След отваряне на USB serial сесия изчакай boot съобщението на ESP32 преди
  provisioning команда; отварянето на serial port може да рестартира платката.

## Relays are out of scope / Релетата не участват в решението

The temporary relay test is complete and has been removed at the owner's request.
Do not add relay commands or automatic relay pulses to EMS firmware.

Временният тест на релетата приключи и е премахнат по искане на собственика.
Не добавяй команди за релета или автоматични импулси в EMS firmware.

## Repository hygiene

- Never commit device-specific credentials, VPN keys, customer addresses,
  production IP ranges, inventory identifiers or USB device paths.
- Never ask a user to send an administrator, SSH or application password in
  chat. When an approved local action needs one, stop at the prompt and ask the
  user to type it directly into the local Terminal or graphical password dialog.
- Maintain English technical documentation with a matching Bulgarian section
  whenever user-facing or operational documentation is changed.
- Record material unfinished deployment work in `HANDOFF.md`, naming this
  repository and the exact next safe action. Update it before ending a
  substantial hardware or deployment task.
- A temporary ESP32 bench connection through the ROCK Pi management Ethernet
  is read-only telemetry testing only. It is never the production OT topology,
  must not be used to reach a BESS or issue device commands, and must be
  removed from the configuration when the isolated OT port is commissioned.
- Before changing firmware, inspect `git status`; preserve unrelated work.

## Mandatory Pull Request workflow / Задължителен Pull Request процес

- Every completed change set must be committed on a named branch, pushed to
  `origin` and given a Pull Request before it is reported as ready for review.
- The Pull Request targets `main` unless an explicitly documented dependency
  requires another base branch. It must state its scope, tests and any
  commissioning limitations.
- Never merge a Pull Request automatically. Report its URL and wait for the
  project owner's review/merge decision.
- If a Pull Request cannot be created because of permissions or a GitHub error,
  report that exact blocker and keep the branch name and commit SHA in
  `CODEX_STATE.md` and `HANDOFF.md` where applicable.

- Всяка завършена промяна се commit-ва в именуван branch, push-ва се към
  `origin` и получава Pull Request, преди да бъде докладвана като готова за
  review.
- Pull Request-ът е към `main`, освен ако изрично документирана зависимост не
  изисква друга base branch. В него се описват обхватът, тестовете и всички
  commissioning ограничения.
- Pull Request не се merge-ва автоматично. Докладва се URL и се изчаква
  review/merge решение на собственика на проекта.
- Ако Pull Request не може да бъде създаден поради права или GitHub грешка,
  докладвай точното препятствие и запиши branch името и commit SHA в
  `CODEX_STATE.md` и при нужда в `HANDOFF.md`.

## Terminology parity / Терминологична синхронизация

For bilingual operational documentation, keep both sections semantically
identical and use the project terms `Site = Обект`, `Edge gateway = Edge шлюз`,
`self-consumption = собствено потребление` and `EFC = еквивалентни пълни
цикли`. Do not translate code identifiers, protocol names or product brands.

За двуезичната оперативна документация поддържай двата раздела семантично
еднакви и използвай термините `Site = Обект`, `Edge gateway = Edge шлюз`,
`self-consumption = собствено потребление` и `EFC = еквивалентни пълни цикли`.
Не превеждай code identifiers, protocol имена или product brands.

## Verified pilot checkpoint / Потвърден пилотен checkpoint

The following results are verified on the current physical pilot and may be
used as evidence by future work; they are not production approval:

- ROCK Pi E completed a native ARM64 build and both repository CTest suites.
- The installed ROCK Pi service is enabled and active, with all write approval
  gates set to `0` and commissioning locked.
- An OLIMEX ESP32-EVB RS485/Ethernet build was flashed over USB at 115200 bps.
- ROCK Pi performed read-only Modbus TCP identity and telemetry reads from the
  ESP32 over the temporary management-LAN bench path.
- The ESP32 driver is still unconfigured. No RS485 downstream command, BESS
  connection or production control action has been enabled.

Тези резултати са потвърдени на текущия физически пилот и могат да се ползват
като доказателство при следваща работа; те не са production одобрение:

- ROCK Pi E е изпълнил native ARM64 build и двата CTest пакета в repository-то.
- Инсталираната ROCK Pi услуга е enabled и active, а всички write approval
  gate-ове са `0` и commissioning е заключен.
- RS485/Ethernet build за OLIMEX ESP32-EVB е flash-нат през USB на 115200 bps.
- ROCK Pi е направил read-only Modbus TCP identity и telemetry четене от ESP32
  през временния management-LAN bench path.
- ESP32 driver-ът все още е unconfigured. Няма разрешена RS485 downstream
  команда, BESS връзка или production control действие.

Before resuming a hardware task, read `HANDOFF.md`, `CODEX_STATE.md` and
`docs/ROCKPI_E_PILOT_STATUS.md`, then inspect the actual repository state.
Преди продължаване на хардуерна задача прочети `HANDOFF.md`, `CODEX_STATE.md`
и `docs/ROCKPI_E_PILOT_STATUS.md`, след което провери реалното състояние на
repository-то.

## Preserved main-branch rules

No public MQTT listener, direct backend route to OT/BESS, or site-to-site routing is permitted.
Update English and Bulgarian operational text in the same commit with identical meaning.
Every HANDOFF must identify its repository directly below the title.

Не се допуска публичен MQTT, директен backend маршрут към OT/BESS или връзка между обекти.
Обновявай EN и BG оперативните текстове в един commit с еднакъв смисъл.
Всеки HANDOFF посочва repository-то непосредствено под заглавието.
