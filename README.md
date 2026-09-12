# GrideX Edge Gateway

## English

Open-source Edge runtime for industrial sites. The selected architecture uses
one Radxa ROCK Pi E controller and OLIMEX ESP32-EVB nodes connected by isolated
OT Ethernet.

| Build | Platform | Role |
|---|---|---|
| `base-rockpie/` | Linux ARM64 / RK3328 | Safety controller, STE-261L driver, node polling and command routing |
| `node-esp32-evb/` | ESP32 / PlatformIO | One device-specific CAN or RS485 driver and local OT Ethernet / Modbus TCP endpoint |

Production nodes use ESP32-EVB-EA-IND. The ordinary ESP32-EVB is a lab option.
The board has Ethernet and CAN; the RS485 profile adds an external galvanically
isolated UEXT/UART transceiver.

### Data paths

```text
TELEMETRY
CAN/RS485 device -> ESP32-EVB -> OT Ethernet / Modbus TCP :1502 -> ROCK Pi E
                -> Site Router WireGuard tunnel -> private MQTT broker
                -> GrideX backend ingestion -> PostgreSQL -> OpenRemote Assets

CONTROL
OpenRemote Strategy/Control Asset -> Site Router WireGuard tunnel
                                  -> ROCK Pi E northbound Modbus TCP :1502
                                  -> OT Ethernet -> ESP32-EVB Modbus TCP :1502
                                  -> CAN/isolated RS485 -> device

NODE COMMAND (where a backend service uses MQTT)
GrideX backend -> private MQTT broker -> Site Router WireGuard tunnel
               -> ROCK Pi E command bridge -> OT Ethernet -> ESP32-EVB

DIRECT BESS
ROCK Pi E -> OT Ethernet -> Suntech STE-261L Modbus TCP 3200
```

ROCK Pi E and ESP32 do not run WireGuard. ROCK Pi E is the sole MQTT bridge for
ESP32 node telemetry and broker commands; ESP32 nodes have no MQTT credentials
in the default production profile.
There is no public MQTT listener, direct ESP32-to-cloud telemetry path, direct
cloud route to the OT/BESS network or MQTT command subscription on a node.
Every node contains one compiled driver for one device type, brand, model and
protocol revision.

Implemented safety includes live BMS limit clamping, software fuse, EMS
command timeout, local Suntech heartbeat, commissioning write lock, a second
driver-side clamp, per-node command sequence/TTL and zero-power fallback.

### Build and tests

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

cmake -S base-rockpie -B build-rockpie
cmake --build build-rockpie
ctest --test-dir build-rockpie --output-on-failure

cd node-esp32-evb
pio run -e esp32-evb-can
pio run -e esp32-evb-rs485
```

The Suntech SunStorage Pro 261 / STE-261L map is manufacturer-confirmed. Other
drivers remain read-only or reference status until their exact hardware and
write behavior pass bench commissioning.

Current pilot status, service ownership and external interface contracts are
documented in [ROCK Pi E pilot status](docs/ROCKPI_E_PILOT_STATUS.md) and
[Services and external interfaces](docs/SERVICES_AND_EXTERNAL_INTERFACES.md).
The temporary read-only ESP32 bench protocol is in
[ROCK Pi E ↔ ESP32-EVB bench contract](docs/ROCKPI_ESP32_BENCH_CONTRACT.md).

## Project credits / Принос към проекта

Created and led by **Dr. Eng. Antuan Hristov Angelov** — product concept, EMS
and system architecture, software development, Edge-gateway design, and
product/UX/UI design. [Digital profile](https://linkmy.cards/en/antouan-anguelov/)
· [LinkedIn](https://www.linkedin.com/in/antouan/) ·
[Българска версия](CREDITS.md#български)

## Български

Това е open-source Edge runtime за индустриални обекти. Избраната архитектура
използва един Radxa ROCK Pi E контролер и OLIMEX ESP32-EVB нодове, свързани по
изолирана OT Ethernet мрежа.

- `base-rockpie/` съдържа Linux услугата, safety логиката, драйвера за
  STE-261L, polling-а на нодовете и маршрутирането на команди.
- `node-esp32-evb/` съдържа firmware за един конкретен CAN или RS485 продукт
  и локален OT Ethernet / Modbus TCP endpoint.

Производственият нод е ESP32-EVB-EA-IND; стандартният ESP32-EVB е за лаборатория.
Платката има Ethernet и CAN. RS485 вариантът добавя външен галванично изолиран
UEXT/UART трансивър.

Телеметрията преминава от нода през Modbus TCP в изолираната OT мрежа до ROCK
Pi E. Само ROCK Pi E я публикува към private MQTT broker през WireGuard тунела
на Site Router, откъдето backend ingestion услугата я записва в PostgreSQL и
синхронизира нужните OpenRemote Assets. Нодовете нямат MQTT credentials в
стандартния production профил. Командите минават през ROCK Pi E и вътрешната
Ethernet мрежа. ROCK Pi E и ESP32 нямат WireGuard, OT/BESS мрежата не се
route-ва към backend, публичен MQTT не се използва и ESP32 не приема MQTT
команди.

Всеки нод се компилира за точно един тип, бранд, модел и протоколна ревизия.
При отпадане на командния TTL нодът подава 0 kW. Локалните BMS лимити,
software fuse, commissioning lock и heartbeat защитите не могат да бъдат
заобиколени от облачната стратегия.

Лиценз: MIT.
