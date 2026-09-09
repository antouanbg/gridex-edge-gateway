# GrideX Edge Gateway

## English

Open-source Edge runtime for industrial sites. The selected architecture uses
one Radxa ROCK Pi E controller and OLIMEX ESP32-EVB nodes connected by isolated
OT Ethernet.

| Build | Platform | Role |
|---|---|---|
| `base-rockpie/` | Linux ARM64 / RK3328 | Safety controller, STE-261L driver, node polling and command routing |
| `node-esp32-evb/` | ESP32 / PlatformIO | One device-specific CAN or RS485 driver, direct MQTT telemetry and local Ethernet control |

Production nodes use ESP32-EVB-EA-IND. The ordinary ESP32-EVB is a lab option.
The board has Ethernet and CAN; the RS485 profile adds an external galvanically
isolated UEXT/UART transceiver.

### Data paths

```text
TELEMETRY
CAN/RS485 device -> ESP32-EVB -> Ethernet -> site router WireGuard
                 -> VPN-only MQTT 8883 -> OpenRemote

CONTROL
OpenRemote -> site router WireGuard -> ROCK Pi E -> OT Ethernet
           -> ESP32-EVB Modbus TCP 1502 -> CAN/isolated RS485 -> device

DIRECT BESS
ROCK Pi E -> OT Ethernet -> Suntech STE-261L Modbus TCP 3200
```

ROCK Pi E and ESP32 do not run WireGuard. They use the site router tunnel.
There is no public MQTT listener, no direct cloud route to the OT/BESS network
and no MQTT command subscription on the node. Every node contains one compiled
driver for one device type, brand, model and protocol revision.

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

## Български

Това е open-source Edge runtime за индустриални обекти. Избраната архитектура
използва един Radxa ROCK Pi E контролер и OLIMEX ESP32-EVB нодове, свързани по
изолирана OT Ethernet мрежа.

- `base-rockpie/` съдържа Linux услугата, safety логиката, драйвера за
  STE-261L, polling-а на нодовете и маршрутирането на команди.
- `node-esp32-evb/` съдържа firmware за един конкретен CAN или RS485 продукт,
  директна MQTT телеметрия и локален Modbus TCP control endpoint.

Производственият нод е ESP32-EVB-EA-IND; стандартният ESP32-EVB е за лаборатория.
Платката има Ethernet и CAN. RS485 вариантът добавя външен галванично изолиран
UEXT/UART трансивър.

Телеметрията отива директно от нода по MQTT/TLS през WireGuard тунела на site
router-а. Командите идват през ROCK Pi E и вътрешната Ethernet мрежа. ROCK Pi E
и ESP32 нямат WireGuard, OT/BESS мрежата не се route-ва към backend, публичен
MQTT не се използва и ESP32 не приема MQTT команди.

Всеки нод се компилира за точно един тип, бранд, модел и протоколна ревизия.
При отпадане на командния TTL нодът подава 0 kW. Локалните BMS лимити,
software fuse, commissioning lock и heartbeat защитите не могат да бъдат
заобиколени от облачната стратегия.

Лиценз: MIT.
