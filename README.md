# GrideX Edge Gateway

Самостоятелен C++20 core за локалния индустриален концентратор. Кодът е отделен от OpenRemote backend и от клиентския уеб интерфейс.

## Реализирано в началния skeleton

- Device driver interface за Modbus устройства.
- Първи драйвер за SunStorage Pro 261 / STE-261L.
- Четене на PCS мощност, SOC, SOH, ток, напрежение, BMS статус и динамични лимити.
- Локален PCS heartbeat на регистри 5301/5302.
- Safety envelope с BMS clamp.
- Software fuse спрямо товар на обекта и договорен лимит.
- Safe mode при загуба на EMS команда.
- Commissioning lock: няма vendor write преди потвърдени address/sign/scale.
- Каноничен northbound регистров договор към OpenRemote.

## Build

~~~bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

Core библиотеката няма външни зависимости. След избор на хардуер се добавят:

1. Реален Modbus TCP/RTU transport adapter, обичайно чрез libmodbus или еквивалент.
2. Modbus TCP slave/server за northbound картата към OpenRemote.
3. Linux systemd service, hardware watchdog и persistent configuration.
4. Platform HAL за RS485 портове, GPIO, RTC и watchdog.

## Power sign

В GrideX положителна мощност означава разряд от батерията, отрицателна означава заряд. Vendor драйверът е единственото място, което може да обръща знака. Този sign convention трябва да бъде потвърден на реалния шкаф преди отключване на командите.

## Source protocol

Началният mapping е извлечен от All-in-one liquid-cooled cabinet BCQ controller Modbus communication, STE-261L, version 2.0, date 2025-09-08. Vendor адресите се пазят в config/sunstorage-pro-261.yaml; northbound картата е отделна и стабилна.

