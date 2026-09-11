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
  ESP32-EVB -> Ethernet -> site router -> site WireGuard tunnel
            -> VPN-only MQTT 8883 -> OpenRemote

Commands
  OpenRemote -> site WireGuard tunnel -> ROCK Pi E
             -> isolated OT Ethernet / Modbus TCP 1502 -> ESP32-EVB
             -> CAN or isolated RS485 -> inverter / BMS / meter / EVSE
```

The node has no WireGuard client. It uses the site router's tunnel. MQTT is
telemetry-only; the firmware does not subscribe to command topics. The local
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
pio run -e esp32-evb-deye-sun100k-g03-rs485
```

`esp32-evb-deye-sun100k-g03-rs485` is the first concrete direct RS-485
telemetry profile. It is for Deye SUN-100K-G03 only and keeps power writes
locked; see [its protocol profile](../docs/DEYE_SUN100K_G03_RS485_PROTOCOL.md).

The concrete driver build adds `GRIDEX_NODE_TYPE` and `GRIDEX_DRIVER_ID`
and links only the selected device driver. Credentials, addresses and
certificates are provisioned after build and are never committed.

### Provisioned namespaces

- `gridex-cloud`: `enabled`, `mqtt_host`, `mqtt_port`, `realm`,
  `mqtt_user`, `mqtt_secret`, `client_id`, `asset_id`, `ca_cert`.
- `gridex-control`: `rockpi_ip`, `port`.
- `gridex-mbus`: `node_type`, `driver_id`.

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

Телеметрията се публикува директно по MQTT/TLS през Ethernet и WireGuard тунела
на site router-а. Командите не идват по MQTT: OpenRemote ги подава към ROCK Pi
E, а той ги изпраща по изолираната OT Ethernet мрежа към Modbus TCP endpoint-а
на нода. Нодът превежда командата към CAN или външния изолиран RS485 канал.

ESP32 няма WireGuard. Modbus TCP сървърът допуска само конфигурирания ROCK Pi E,
само командните регистри, последователен sequence и TTL 1–30 секунди. При
изтичане на TTL се изпраща безопасна команда 0 kW.

CAN профилът използва официалните OLIMEX GPIO5/GPIO35. RS485 профилът използва
UEXT UART GPIO4/GPIO36 и конфигурируем direction GPIO; конкретната carrier
платка, изолацията, терминирането и защитите трябва да се валидират електрически.

`esp32-evb-deye-sun100k-g03-rs485` е първият конкретен RS485 telemetry профил
за Deye SUN-100K-G03. Командите за мощност в него са заключени; вижте
[протоколния профил](../docs/DEYE_SUN100K_G03_RS485_PROTOCOL.md).

Източници:

- [OLIMEX ESP32-EVB repository](https://github.com/OLIMEX/ESP32-EVB)
- [OLIMEX ESP32-EVB product page](https://www.olimex.com/Products/IoT/ESP32/ESP32-EVB-EA-IND/open-source-hardware)
- [PlatformIO board definition](https://docs.platformio.org/en/stable/boards/espressif32/esp32-evb.html)
