# PR #3 technical review / Технически преглед

Repository / GitHub: antouanbg/gridex-edge-gateway

## English

Review date: 2026-09-12. Scope: the Edge source and documentation changes in
PR #3, including the owner's request to remove the temporary relay test.

### Findings resolved

- Removed the entire relay test project, automatic startup pulses and unused
  relay pin constants. Relays are outside the EMS scope.
- Fixed serial line overflow: discard the whole overlong line, not just its
  prefix. Limit serial processing to 80 bytes per loop to preserve polling.
  Report NVS save failure rather than claiming the new source was persisted.
- Disabled direct ESP32 MQTT even when legacy NVS contains an enabled flag.
  Corrected the node README, bench diagram and service table to show ROCK Pi
  as the planned MQTT publisher. Its publisher remains separate pending work.
- Reject FC04 reads that cross the 16-bit register-address boundary.
- Retained the current main-branch data paths and documentation rules when
  resolving the overlap with the merged documentation PR #4.

### Verification performed for this review

- Fresh Debug CMake build of the base and core: all 3 CTest suites pass.
- Fresh Debug CMake build of the node: its CTest suite passes.
- The new loopback TCP simulator verifies one FC04 request for 122–125,
  high-word-first words, fragmented replies, malformed byte counts, Modbus
  exceptions and wrong function codes. Invalid ranges are rejected.
- Core tests reject individual energy-word reads and verify that a failed
  atomic read clears extended telemetry validity.
- Node tests cover complete/fragmented serial lines, CRLF, length limits and
  an overlong line containing a command-like suffix.
- PlatformIO builds of both CAN and RS485 profiles pass. No upload was made.
- The initial sandbox prevented local socket binds and toolchain-cache writes;
  the same checks passed after granting those local test permissions.
- Diff formatting and changed-file secret checks passed.

### Operational limits retained

This is a reviewed pilot foundation, not production commissioning. Current
node builds instantiate UnconfiguredDriver and reject device commands.
Before adding a concrete driver, validate the complete command lifecycle
(disable/reject must stop prior output, TTL expiry/replay and reconnect),
vendor maps and electrical wiring. The ROCK Pi MQTT publisher, deployment
credentials and telemetry soak remain handoff work. No live device tests or
service activation were performed during this review.

## Български

Дата: 2026-09-12. Обхват: Edge кодът и документацията в PR #3, включително
искането на собственика да се премахне временният тест за релета.

### Отстранени проблеми

- Премахнати са целият проект за тест на релета, автоматичните импулси при
  старт и неизползваните константи за пиновете. Релетата са извън EMS обхвата.
- При препълнен сериен ред се отхвърля целият ред, включително суфиксът.
  Обработват се до 80 байта на итерация, за да продължава polling-ът.
  При неуспешен NVS запис се съобщава грешка, вместо потвърждение за запис.
- Директният ESP32 MQTT е изключен дори при стар enabled флаг в NVS.
  README на нода, bench схемата и таблицата с услуги показват ROCK Pi като
  планиран MQTT publisher. Този publisher остава отделна задача.
- FC04 четене извън 16-битовия адресен диапазон се отказва.
- При разрешаване на припокриването с PR #4 са запазени актуалните data paths
  и документационните правила от main.

### Проверки в този review

- Нов Debug CMake build на base/core: и трите CTest пакета минават.
- Нов Debug CMake build на нода: неговият CTest пакет минава.
- Новият локален TCP симулатор проверява една FC04 заявка за 122–125,
  high-word-first думи, фрагментирани отговори, грешен byte count, Modbus
  exceptions и грешен function code. Невалидните диапазони се отказват.
- Core тестовете забраняват отделно четене на energy думи и проверяват, че
  неуспешното атомарно четене изключва extended telemetry validity.
- Node тестовете покриват цели/фрагментирани серийни редове, CRLF, лимита за
  дължина и препълнен ред със суфикс, приличащ на команда.
- PlatformIO build-овете за CAN и RS485 минават. Не е качван firmware.
- Първоначалният sandbox блокира локални socket bind и toolchain-cache запис;
  същите проверки минаха след разрешаване на локалния достъп за тестове.
- Проверките на diff форматирането и за тайни в промените минаха.

### Запазени оперативни ограничения

Това е прегледана пилотна основа, а не production commissioning. Текущите
node build-ове използват UnconfiguredDriver и отказват команди към устройства.
Преди конкретен driver трябва да се валидират disable/reject на предходен
изход, TTL/replay/reconnect, vendor картите и окабеляването. MQTT publisher-ът
на ROCK Pi, deployment credentials и telemetry soak остават в HANDOFF.
При този review не са тествани живи устройства и не са включвани услуги.
