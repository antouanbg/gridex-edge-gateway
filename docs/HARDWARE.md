# GrideX Edge hardware baseline V2

## English

### Controller

**Radxa ROCK Pi E / RK3328**, preferably 2 GB RAM and industrial-grade storage.
Its two physical Ethernet interfaces have fixed roles:

- CONTROL/WAN: site router, private backend services, time and updates.
- OT: STE-261L and OLIMEX nodes through an industrial switch; no default route.

Linux forwarding is disabled. The firewall does not bridge CONTROL and OT.
The enclosure requires a protected 24 V input, watchdog, RTC, DIN mounting and
site-appropriate environmental certification.

### Local OTA update service

ROCK Pi E hosts the outbound-only `gridex_ota_apply` operator client. ESP32
nodes expose their OTA upload endpoint only on local Ethernet after serial
provisioning of a ROCK Pi source and a per-node token verifier. Site Router is
the WireGuard endpoint and never forwards OTA traffic directly to ESP32.

### Only supported programmable node family

**OLIMEX ESP32-EVB**:

- production: ESP32-EVB-EA-IND, external antenna, −40…+85°C;
- laboratory: ESP32-EVB, 0…70°C;
- 100 Mbps Ethernet;
- onboard CAN transceiver, TX GPIO5 / RX GPIO35;
- optional isolated RS485 carrier on UEXT UART, TX GPIO4 / RX GPIO36;
- one node = one device type + brand + model + driver build.

RS485 is not onboard. The carrier must provide galvanic isolation, isolated
power, DE/RE control, termination/bias and ESD/surge protection. The direction
GPIO is carrier-specific and cannot be frozen before schematic validation.

### Remaining production decisions

1. Exact ROCK Pi E RAM/storage and Linux image.
2. Managed OT switch, VLAN/firewall rules and connector standard.
3. RS485 isolator, isolation rating, surge level and termination.
4. 24 V/5 V power architecture, short backup and watchdog.
5. DIN enclosure, IP class, temperature and certification targets.
6. First-site I/O inventory and cable lengths.

## Български

Управляващият модул е Radxa ROCK Pi E с два отделни Ethernet порта:
CONTROL/WAN към site router-а и OT към STE-261L и ESP32-EVB нодовете.
IP forwarding е изключен и OT няма default route.

### Локална OTA услуга за обновяване

ROCK Pi E предоставя операторския, само outbound `gridex_ota_apply` client.
ESP32 нодовете предоставят OTA upload endpoint само по локален Ethernet след
serial provisioning на ROCK Pi source и verifier за token на отделния нод.
Site Router е WireGuard endpoint и никога не препраща OTA трафик директно към
ESP32.

Единствената програмируема фамилия нодове е OLIMEX ESP32-EVB. За производство
се препоръчва ESP32-EVB-EA-IND (−40…+85°C), а стандартният вариант е за тестове.
Платката има Ethernet и CAN на GPIO5/GPIO35. За RS485 се добавя отделен
галванично изолиран трансивър към UEXT UART GPIO4/GPIO36.

Всеки нод обслужва един физически продукт и съдържа само неговия компилиран
драйвер. Изолацията, захранването, DE/RE GPIO, терминирането и surge/ESD
защитата на RS485 carrier-а се финализират със схемата и BOM-а.

Sources / Източници:

- [OLIMEX ESP32-EVB](https://github.com/OLIMEX/ESP32-EVB)
- [ESP32-EVB-EA-IND](https://www.olimex.com/Products/IoT/ESP32/ESP32-EVB-EA-IND/open-source-hardware)
- [PlatformIO ESP32-EVB](https://docs.platformio.org/en/stable/boards/espressif32/esp32-evb.html)
