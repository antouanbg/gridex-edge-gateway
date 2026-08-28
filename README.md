# GrideX Edge Gateway

Самостоятелен Edge проект, отделен от OpenRemote backend и клиентския интерфейс. Хардуер Rev A: Radxa ROCK Pi E + LilyGo T-CAN485 нодове.

Проектът е open source и се разпространява под MIT License.

## Два независими build-а

| Проект | Платформа | Роля |
|---|---|---|
| `base-rockpie/` | Linux ARM64 / RK3328 | STE-261L Modbus TCP, heartbeat, safety envelope, WAN/OT separation и MBUS master |
| `node-tcan485/` | ESP32 / PlatformIO | Modbus RTU server, UID/addressing, канонична карта и vendor driver |

Общият C++20 safety/STE core остава в `include/` и `src/` и се използва от Linux build-а.

## Реализирано

- Device driver interface за Modbus устройства.
- Първи драйвер за SunStorage Pro 261 / STE-261L.
- Четене на PCS мощност, SOC, SOH, ток, напрежение, BMS статус и динамични лимити.
- Локален PCS heartbeat на регистри 5301/5302.
- Safety envelope с BMS clamp.
- Втори независим clamp в vendor драйвера: директна команда над последните валидни BMS лимити се отказва.
- Опционални site/PCS caps за заряд и разряд, които могат само да намалят BMS envelope-а.
- Software fuse спрямо товар на обекта и договорен лимит.
- Safe mode при загуба на EMS команда.
- Commissioning lock: няма vendor write преди потвърдени address/sign/scale.
- Каноничен northbound регистров договор към OpenRemote.
- Работещ Modbus TCP northbound server на порт `1502` с FC03/04/06/16, command sequence и EMS heartbeat.

## Core build и тестове

~~~bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

Core библиотеката няма външни зависимости. След избор на хардуер се добавят:

1. `base-rockpie` предоставя POSIX Modbus TCP transport и systemd service.
2. `node-tcan485` предоставя самостоятелен ESP32 firmware и host тестове на MBUS протокола.
3. Конкретните meter/EVSE register maps се добавят след получаване и валидиране на протоколите на устройствата.

## Power sign

В GrideX положителна мощност означава разряд от батерията, отрицателна означава заряд. Производителят потвърди знака и коефициента ×10; на реалния шкаф остава задължителен ограничен commissioning тест преди отключване на командите.

## Source protocol

Началният mapping е извлечен от All-in-one liquid-cooled cabinet BCQ controller Modbus communication, STE-261L, version 2.0, date 2025-09-08. Vendor адресите се пазят в config/sunstorage-pro-261.yaml; northbound картата е отделна и стабилна.
