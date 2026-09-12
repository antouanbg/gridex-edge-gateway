# OLIMEX ESP32-EVB device node

## English

This is the only programmable node family in GrideX Edge V2. The production
choice is **ESP32-EVB-EA-IND**; ESP32-EVB is supported for laboratory work.
Both provide wired 100 Mbps Ethernet and an onboard CAN transceiver. RS485
requires a separate galvanically isolated transceiver connected to UEXT/UART.

Each node is built for exactly one equipment type, manufacturer, model and
protocol revision. Mixed products on one node are not supported.

### Communication paths

```text
Sensors / device telemetry
  ESP32-EVB -> OT Modbus TCP -> ROCK Pi E
            -> Site Router VPN -> private MQTT -> backend ingestion -> OpenRemote

Commands
  OpenRemote -> site WireGuard tunnel -> ROCK Pi E
             -> isolated OT Ethernet / Modbus TCP 1502 -> ESP32-EVB
             -> CAN or isolated RS485 -> inverter / BMS / meter / EVSE
```

The node exposes telemetry for ROCK Pi polling. Direct node MQTT is disabled,
even with a legacy enabled flag in NVS. ROCK Pi publishes normalized telemetry
through its outbound-only private MQTT bridge. The local
Modbus TCP control endpoint accepts clients only from the configured ROCK Pi E
address, permits writes only to the command register window, requires a
monotonic sequence and a 1–30 second TTL, and writes a zero-power safe command
when the TTL expires.

### Hardware profiles

- `esp32-evb-can`: onboard CAN, official OLIMEX pins TX GPIO5 / RX GPIO35.
- `esp32-evb-rs485`: UEXT UART TX GPIO4 / RX GPIO36 plus an external isolated
  half-duplex transceiver; direction defaults to GPIO13 and must be validated
  against the selected carrier.
- Ethernet startup uses the OLIMEX-recommended delay before `ETH.begin()`.

The CAN environment follows the official OLIMEX TWAI example. The RS485
transceiver, isolation voltage, termination, bias, ESD/surge protection and
field connector belong to the production carrier/BOM and require electrical
validation.

### Build

```bash
pio run -e esp32-evb-can
pio run -e esp32-evb-rs485
```

Both current profiles instantiate only UnconfiguredDriver and reject all power
commands. A concrete vendor driver and its commissioning tests are still needed.
Setting a type or driver ID does not implement a driver. Local source addresses
are provisioned after build and are never committed.

### Provisioning and health

The isolated OT interface uses DHCP. The ROCK Pi protected deployment
environment and Site Router reservation are authoritative for stable node
addressing; the firmware does not contain a static address. Local USB serial
provisioning accepts `node <1-247> <type 1-6> <driver-id>` and persists the
identity in `gridex-mbus`; `status` reports Ethernet, Modbus listener, device
bus and watchdog state. This remains an identity-only action until a matching
signed driver build exists.

Canonical map version 5 adds read-only health registers `0x0046–0x004E` for
Ethernet, Modbus listener, driver/bus/watchdog readiness, recovery counters and
the latest local fault code. ROCK Pi polls this block when it is available and
records it in normalized telemetry. Ethernet recovery restarts the local
listener after the node receives IP again. A future compiled driver may opt
into device-bus recovery; `UnconfiguredDriver` intentionally never performs a
RS485/CAN request.

### Local OTA updates

OTA is disabled by default. It is a local ROCK Pi-to-node procedure, not a
VPN, public Internet or direct MQTT feature. The ESP32 accepts a multipart
`POST /gridex/ota` on TCP 8080 only from its provisioned ROCK Pi address and
only when a per-node SHA-256 token verifier and the firmware SHA-256 match.
The node stores the verifier, never the reusable token. Provision with the
local serial commands `rockpi <IPv4>` and `ota-key <32+ chars>`; see
[ESP32 OTA](../docs/ESP32_OTA.md) for the complete EN/BG procedure.

### Provisioned namespaces

- `gridex-cloud`: legacy namespace only; cannot activate MQTT in these builds.
- `gridex-control`: `rockpi_ip`, `port`, `ota_port`, `ota_token_hash`.
- `gridex-mbus`: `node_address`, `node_type`, `driver_id`.

No public MQTT listener is supported. In production the broker address must be
reachable only through the site router's VPN path.

## Български

Това е единствената програмируема фамилия нодове в GrideX Edge V2.
Препоръчителният производствен вариант е **ESP32-EVB-EA-IND**, а ESP32-EVB се
поддържа за лабораторни тестове. Платката има 100 Mbps Ethernet и вграден CAN
трансивър. За RS485 е необходим отделен галванично изолиран трансивър към
UEXT/UART.

Всеки нод се компилира за точно един тип устройство, производител, модел и
ревизия на протокола. Смесени продукти върху един нод не се разрешават.

ROCK Pi чете телеметрията на ESP32 по OT Modbus TCP. Планираното препращане
е ROCK Pi → Site Router VPN → private MQTT → backend ingestion → OpenRemote.
Директният MQTT от ESP32 е изключен дори при стар enabled флаг в NVS. ROCK Pi
препраща нормализираната телеметрия през outbound-only private MQTT bridge.
Командите не идват по MQTT: OpenRemote ги подава към ROCK Pi E, а
той ги изпраща по изолираната OT Ethernet мрежа към Modbus TCP endpoint-а на
нода. Нодът превежда командата към CAN или външния изолиран RS485 канал.

ESP32 няма WireGuard. Modbus TCP сървърът допуска само конфигурирания ROCK Pi E,
само командните регистри, последователен sequence и TTL 1–30 секунди. При
изтичане на TTL се изпраща безопасна команда 0 kW.

CAN профилът използва официалните OLIMEX GPIO5/GPIO35. RS485 профилът използва
UEXT UART GPIO4/GPIO36 и конфигурируем direction GPIO; конкретната carrier
платка, изолацията, терминирането и защитите трябва да се валидират електрически.

И двата текущи профила използват само UnconfiguredDriver и отказват всички
power команди. Изборът на type или driver ID не имплементира vendor driver.
Необходими са конкретен драйвер и commissioning тестове. NVS използва
`gridex-control` (`rockpi_ip`, `port`, `ota_port`, `ota_token_hash`) и `gridex-mbus` (`node_address`,
`node_type`, `driver_id`); `gridex-cloud` е неактивна наследена конфигурация.

### Provisioning и health

Изолираният OT интерфейс използва DHCP. Защитеният deployment environment на
ROCK Pi и Site Router reservation са авторитетни за устойчивото node
адресиране; firmware-ът не съдържа статичен адрес. Local USB serial
provisioning приема `node <1-247> <type 1-6> <driver-id>` и записва
идентичността в `gridex-mbus`; `status` показва Ethernet, Modbus listener,
device bus и watchdog състояние. Това остава действие само за идентичност,
докато няма съвпадащ signed driver build.

Canonical map version 5 добавя read-only health регистрите `0x0046–0x004E` за
Ethernet, Modbus listener, готовност на driver/bus/watchdog, recovery броячи и
последния local fault код. ROCK Pi poll-ва блока, когато е наличен, и го
записва в нормализираната telemetry. Ethernet recovery рестартира local
listener-а след като нодът отново получи IP. Бъдещ компилиран driver може да
се включи към device-bus recovery; `UnconfiguredDriver` умишлено не изпраща
RS485/CAN заявка.

### Локални OTA обновявания

OTA е изключено по подразбиране. То е локална процедура ROCK Pi към нод, а не
VPN, публичен Интернет или direct MQTT функционалност. ESP32 приема multipart
`POST /gridex/ota` на TCP 8080 само от provision-натия адрес на ROCK Pi и само
ако SHA-256 token verifier-ът за нода и SHA-256 на firmware-а съвпадат. Нодът
записва verifier-а, а не повторно използваемия token. Provision-ването е през
локалните serial команди `rockpi <IPv4>` и `ota-key <32+ chars>`; пълната EN/BG
процедура е в [ESP32 OTA](../docs/ESP32_OTA.md).

Източници:

- [OLIMEX ESP32-EVB repository](https://github.com/OLIMEX/ESP32-EVB)
- [OLIMEX ESP32-EVB product page](https://www.olimex.com/Products/IoT/ESP32/ESP32-EVB-EA-IND/open-source-hardware)
- [PlatformIO board definition](https://docs.platformio.org/en/stable/boards/espressif32/esp32-evb.html)
