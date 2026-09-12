# GrideX Edge Gateway

## English

Open-source Edge runtime for industrial sites. The selected architecture uses
one Radxa ROCK Pi E controller and OLIMEX ESP32-EVB nodes connected by isolated
OT Ethernet.

| Build | Platform | Role |
|---|---|---|
| `base-rockpie/` | Linux ARM64 / RK3328 | Safety controller, STE-261L driver, node polling and command routing |
| `node-esp32-evb/` | ESP32 / PlatformIO | One device-specific CAN or RS485 driver and local OT Ethernet / Modbus TCP endpoint |
| `gridex_ota_apply` + ESP32 OTA endpoint | ROCK Pi E + ESP32 | Protected local firmware-update service: ROCK Pi sends a verified image to one ESP32 node; it is neither a WireGuard nor public service |

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

DIRECT BESS
ROCK Pi E -> OT Ethernet -> Suntech STE-261L Modbus TCP 3200
```

ROCK Pi E and ESP32 do not run WireGuard. ROCK Pi E is the sole MQTT bridge for
ESP32 node telemetry and Edge health; ESP32 nodes have no MQTT credentials in
the default production profile.
There is no public MQTT listener, direct ESP32-to-cloud telemetry path, direct
cloud route to the OT/BESS network or MQTT command subscription on a node.
Every node contains one compiled driver for one device type, brand, model and
protocol revision.

### OTA update service

The Edge runtime has a separate OTA update service in addition to telemetry and
control. `gridex_ota_apply` is an operator-invoked, outbound-only client on
ROCK Pi E; the ESP32 exposes a protected local upload endpoint only after local
provisioning. The image and per-node secret are verified by SHA-256. This path
is `ROCK Pi E → local Ethernet → ESP32`, never WireGuard or public Internet.
See [ESP32 OTA](docs/ESP32_OTA.md).

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
The reconciled pilot evidence and the required no-backend work order are in
[Live hardware status](docs/LIVE_HARDWARE_STATUS.md) and
[Local commissioning sequence](docs/LOCAL_COMMISSIONING_SEQUENCE.md).
The persistent, per-site node address and ROCK Pi endpoint procedure is in
[Node network provisioning](docs/NODE_NETWORK_PROVISIONING.md).
The isolated dual-Ethernet DHCP procedure is in
[ROCK Pi isolated OT DHCP](docs/OT_DHCP.md).
The bounded offline journal and node provisioning/health contract are in
[Local telemetry journal and ESP32 node provisioning](docs/LOCAL_TELEMETRY_AND_NODE_PROVISIONING.md).

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
- `gridex_ota_apply` + ESP32 OTA endpoint са защитената локална услуга за
  firmware обновяване: ROCK Pi подава проверен образ към един ESP32 нод; това
  не е WireGuard или публична услуга.

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

### OTA услуга за обновяване

Освен telemetry и control, Edge runtime-ът има отделна OTA услуга за
обновяване. `gridex_ota_apply` е операторски, само outbound client на ROCK Pi
E; ESP32 предоставя защитен локален upload endpoint само след local
provisioning. Образът и тайната за отделния нод се проверяват чрез SHA-256.
Пътят е `ROCK Pi E → локален Ethernet → ESP32`, никога WireGuard или публичен
Интернет. Виж [ESP32 OTA](docs/ESP32_OTA.md).

Лиценз: MIT.

Съгласуваните pilot доказателства и задължителният ред за работа без backend
са в [Текущ хардуерен статус](docs/LIVE_HARDWARE_STATUS.md) и
[Последователност за локален commissioning](docs/LOCAL_COMMISSIONING_SEQUENCE.md).
Устойчивата per-site процедура за адреса на нода и endpoint-а на ROCK Pi е в
[Мрежово provision-ване на нод](docs/NODE_NETWORK_PROVISIONING.md).
Процедурата за изолирания dual-Ethernet DHCP е в
[ROCK Pi изолиран OT DHCP](docs/OT_DHCP.md).
Ограниченият offline журнал и договорът за node provisioning/health са в
[Локален telemetry журнал и ESP32 node provisioning](docs/LOCAL_TELEMETRY_AND_NODE_PROVISIONING.md).
