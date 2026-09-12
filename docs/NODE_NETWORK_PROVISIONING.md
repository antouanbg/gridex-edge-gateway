# Node network provisioning / Мрежово provision-ване на нод

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

### Purpose

Each ESP32-EVB node must retain a stable local Modbus TCP endpoint so that
ROCK Pi polling, node health and the local OTA service recover after reboot.
This is per-site commissioning, not a task repeated at every device reboot.

### Authoritative locations

| Data | Owner and location | Persistence |
|---|---|---|
| ESP32 network address | Site Router DHCP reservation, keyed by the node Ethernet MAC | Router configuration; survives ESP32 reboot/reflash unless the reservation is removed |
| ROCK Pi node endpoint list | Root-owned `/etc/gridex/gridex-rockpie.env`, `GRIDEX_NODE_ENDPOINTS` | Local ROCK Pi storage; systemd reads it at service start |
| ESP32 trusted ROCK Pi source | ESP32 NVS `gridex-control` key `rockpi_ip` | ESP32 flash NVS; survives reboot and ordinary firmware updates |
| ESP32 node identity | ESP32 NVS `gridex-mbus` keys for node address, type and driver ID | ESP32 flash NVS; one logical device per node |
| OTA verifier | ESP32 NVS `gridex-control` key `ota_token_hash` | SHA-256 verifier only; the reusable secret never goes into NVS or Git |

No real addresses, MAC addresses, secrets or customer inventory belong in Git.
The repository contains only `base-rockpie/config/gridex-rockpie.env.example`
and generic names such as `node-01.ot.internal`.

### First-site and replacement procedure

1. Create one Site Router DHCP reservation per ESP32 MAC address. Give the
   node a stable OT hostname/address; do not rely on a changing DHCP lease.
2. Assign ROCK Pi a stable OT address without a default gateway. It is the
   only source accepted by ESP32 Modbus control and OTA.
3. During local USB serial provisioning of ESP32, enter
   `rockpi <ROCK_PI_OT_ADDRESS>` and, when OTA is needed,
   `ota-key <RANDOM_32_TO_128_CHARACTER_SECRET>`. Do not send these commands
   through MQTT, WireGuard or a browser.
4. On ROCK Pi, an administrator records the stable node endpoint in the
   root-owned environment file, for example:

   ```ini
   GRIDEX_NODE_ENDPOINTS=node-01.ot.internal:1502
   GRIDEX_NODE_TIMEOUT_MS=400
   GRIDEX_NODE_POLL_MS=500
   ```

5. The administrator validates the syntax, restarts only
   `gridex-rockpie.service`, and confirms the loopback northbound node slot is
   online. All `GRIDEX_APPROVE_*` values remain `0`.
6. Record the site configuration revision in the deployment system. Do not put
   actual endpoint values into a generic Git branch, firmware source, chat log
   or backend browser response.

### Operational rules

- A normal reboot requires no reprovisioning. ROCK Pi reloads its environment
  file and ESP32 retains its NVS values.
- Repeat this procedure only when a node is replaced, its MAC/address changes,
  the OT network is redesigned, or ROCK Pi's OT address changes.
- If the node is temporarily on a bench LAN, its endpoint is temporary. Before
  production, move it to the reserved OT identity and update the root-owned
  ROCK Pi file in the same commissioning change.
- Site Router remains the WireGuard endpoint. VPN policy may reach ROCK Pi
  management only; it must not route directly to ESP32, Modbus TCP or OTA.

### Current pilot confirmation

The current pilot restored the ESP32 stable address after a DHCP-address change.
An administrator verified the protected `GRIDEX_NODE_ENDPOINTS` value and
restarted the locked read-only service. The ROCK Pi normalized node slot is now
online; the local Modbus TCP listener, the ESP32 Modbus TCP path and the
non-listening OTA client all respond. No BESS/PCS or node control command was
sent. This is configuration recovery evidence, not production-control
acceptance.

## Български

### Предназначение

Всеки ESP32-EVB нод трябва да има устойчив локален Modbus TCP endpoint, за да
се възстановяват ROCK Pi polling-ът, node health и локалната OTA услуга след
рестарт. Това е commissioning за всеки Обект, а не действие при всеки рестарт
на устройство.

### Авторитетни места за съхранение

| Данни | Собственик и място | Устойчивост |
|---|---|---|
| Мрежов адрес на ESP32 | DHCP reservation в Site Router по Ethernet MAC на нода | Router конфигурация; остава след ESP32 рестарт/reflash, освен ако reservation-ът не бъде премахнат |
| Списък с node endpoint-и на ROCK Pi | Root-owned `/etc/gridex/gridex-rockpie.env`, `GRIDEX_NODE_ENDPOINTS` | Локално съхранение на ROCK Pi; systemd го чете при старт на услугата |
| Доверен ROCK Pi source на ESP32 | ESP32 NVS `gridex-control`, ключ `rockpi_ip` | ESP32 flash NVS; остава след рестарт и обикновено firmware обновяване |
| Идентичност на ESP32 нода | ESP32 NVS `gridex-mbus`, ключове за node address, type и driver ID | ESP32 flash NVS; един логически продукт на нод |
| OTA verifier | ESP32 NVS `gridex-control`, ключ `ota_token_hash` | Само SHA-256 verifier; повторно използваемата тайна не влиза в NVS или Git |

В Git не принадлежат реални адреси, MAC адреси, тайни или клиентски inventory.
Repository-то съдържа само `base-rockpie/config/gridex-rockpie.env.example` и
общи имена като `node-01.ot.internal`.

### Процедура за първи Обект и подмяна

1. Създай един Site Router DHCP reservation за всеки ESP32 MAC адрес. Дай на
   нода устойчив OT hostname/address; не разчитай на променящ се DHCP lease.
2. Задай на ROCK Pi устойчив OT адрес без default gateway. Той е единственият
   source, приеман от ESP32 за Modbus control и OTA.
3. При local USB serial provisioning на ESP32 въведи
   `rockpi <ROCK_PI_OT_ADDRESS>` и, когато е нужна OTA,
   `ota-key <RANDOM_32_TO_128_CHARACTER_SECRET>`. Не подавай тези команди през
   MQTT, WireGuard или browser.
4. На ROCK Pi администратор записва устойчивия node endpoint в root-owned
   environment файла, например:

   ```ini
   GRIDEX_NODE_ENDPOINTS=node-01.ot.internal:1502
   GRIDEX_NODE_TIMEOUT_MS=400
   GRIDEX_NODE_POLL_MS=500
   ```

5. Администраторът валидира синтаксиса, рестартира само
   `gridex-rockpie.service` и потвърждава, че loopback northbound node slot е
   online. Всички `GRIDEX_APPROVE_*` стойности остават `0`.
6. Запиши revision на site конфигурацията в deployment системата. Не слагай
   реални endpoint стойности в общ Git branch, firmware source, chat log или
   backend browser response.

### Оперативни правила

- Нормален рестарт не изисква ново provision-ване. ROCK Pi зарежда отново
  environment файла, а ESP32 запазва NVS стойностите си.
- Повтори тази процедура само при подмяна на нод, промяна на MAC/address,
  redesign на OT мрежата или промяна на OT адреса на ROCK Pi.
- Ако нодът е временно в bench LAN, endpoint-ът му е временен. Преди
  production го премести към резервираната OT идентичност и обнови root-owned
  файла на ROCK Pi в същата commissioning промяна.
- Site Router остава WireGuard endpoint. VPN policy може да достига само
  management-а на ROCK Pi; не трябва да route-ва директно до ESP32, Modbus TCP
  или OTA.

### Текущо потвърждение от pilot

Текущият pilot възстанови устойчивия адрес на ESP32 след промяна на DHCP
адрес. Администратор провери защитената `GRIDEX_NODE_ENDPOINTS` стойност и
рестартира заключената read-only услуга. Нормализираният ROCK Pi node slot вече
е online; отговарят локалният Modbus TCP listener, ESP32 Modbus TCP пътят и
OTA client-ът без listener. Не е изпратена команда към BESS/PCS или нод. Това
е доказателство за възстановена конфигурация, а не приемане на production
управление.
