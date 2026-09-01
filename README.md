# GrideX Edge Gateway

## English

Standalone Edge project separated from the OpenRemote backend and the customer interface. Rev A hardware: Radxa ROCK Pi E with LilyGo T-CAN485 nodes.

The project is open source under the MIT License.

### Two independent builds

| Project | Platform | Role |
|---|---|---|
| `base-rockpie/` | Linux ARM64 / RK3328 | STE-261L Modbus TCP, heartbeat, safety envelope, WAN/OT separation and MBUS master |
| `node-tcan485/` | ESP32 / PlatformIO | Modbus RTU server, UID/addressing, canonical map and vendor driver |

The shared C++20 safety/STE core remains in `include/` and `src/` and is used by the Linux build.

### Implemented

- Device-driver interface for Modbus equipment.
- Initial SunStorage Pro 261 / STE-261L driver.
- PCS power, SOC, SOH, current, voltage, BMS state and dynamic-limit reads.
- Local PCS heartbeat on registers 5301/5302.
- Safety envelope with BMS clamping.
- A second independent clamp in the vendor driver rejects any direct command above the latest valid BMS limits.
- Optional site/PCS charge and discharge caps that may only reduce the BMS envelope.
- Software fuse based on site load and contracted power.
- Safe mode after loss of the EMS command.
- Commissioning lock: no vendor write before address, sign and scaling are confirmed.
- Canonical northbound register contract for OpenRemote.
- Working Modbus TCP northbound server on port `1502` with FC03/04/06/16, command sequencing and EMS heartbeat.
- Continuous MBUS polling for up to 32 meter/EVSE/inverter/BMS nodes over RS485.
- Sixteen normalized input registers per node starting at address `0x0100`.
- Independent direct MQTTS telemetry from ESP32 nodes to private-cloud OpenRemote.
- Extended STE-261L telemetry: PCS state, DC/reactive power, setpoint, frequency, accumulated/daily charge-discharge energy and alarm summary.
- Protected operator-only channel for start/stop, reactive power and SOC limits, using an apply key, separate sequence and result code.

### Communication paths

```text
Device <- Modbus RTU -> ESP32 node <- RS485 MBUS -> ROCK Pi E
                                \- MQTTS 8883 -> OpenRemote private cloud

OpenRemote -> Modbus TCP 1502 -> ROCK Pi E -> safety/driver -> device
```

The RS485 path never depends on the internet and is mandatory for local protection, software fuse and control. The MQTTS path is for direct telemetry only; ESP32 nodes do not accept cloud commands.

### Core build and tests

~~~bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

The core library has no external dependencies. After the hardware is selected:

1. `base-rockpie` provides the POSIX Modbus TCP transport and systemd service.
2. `node-tcan485` provides standalone ESP32 firmware and host-side MBUS protocol tests.
3. Exact meter/EVSE register maps are added only after their protocol documents are received and validated.

### Power sign

In GrideX, positive power means battery discharge and negative power means charge. The manufacturer confirmed the sign and ×10 coefficient; a limited commissioning test on the real cabinet remains mandatory before commands are unlocked.

### Source protocol

The initial mapping is derived from *All-in-one liquid-cooled cabinet BCQ controller Modbus communication*, STE-261L, version 2.0, dated 2025-09-08. Vendor addresses are stored in `config/sunstorage-pro-261.yaml`; the northbound map remains separate and stable.

---

## Български

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
- Непрекъснат MBUS polling service за до 32 meter/EVSE/inverter/BMS нода по RS485.
- По 16 нормализирани input регистъра за всеки нод от адрес `0x0100` нагоре.
- Независима директна MQTTS телеметрия от ESP32 нодовете към private-cloud OpenRemote.
- Разширена STE-261L телеметрия: PCS status, DC/reactive power, setpoint,
  frequency, accumulated/daily charge-discharge energy и alarm summary.
- Защитен operator-only канал за start/stop, reactive power и SOC limits с
  apply key, отделен sequence и result code.

## Комуникационни пътища

```text
Устройство <- Modbus RTU -> ESP32 node <- RS485 MBUS -> ROCK Pi E
                                  \- MQTTS 8883 -> OpenRemote private cloud

OpenRemote -> Modbus TCP 1502 -> ROCK Pi E -> safety/driver -> устройство
```

RS485 пътят никога не зависи от интернет и е задължителен за локални защити,
software fuse и управление. MQTTS пътят е само за директна телеметрия; облачни
команди не се приемат от ESP32 нода.

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
