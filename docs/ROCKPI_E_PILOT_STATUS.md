# ROCK Pi E pilot deployment status

## English

This record describes the first physical ROCK Pi E pilot only. It contains no
customer network addresses, credentials, VPN details or production endpoints.

### Completed

- Armbian Minimal CLI pilot image was written to removable storage after hash
  verification and booted successfully on the ROCK Pi E.
- The board exposes both Ethernet controllers. The management interface has a
  DHCP lease through the Site Router; the OT interface is present but has no
  carrier and no configured route.
- CMake and the C++20 toolchain were installed on the board.
- `gridex_rockpie_service` was built natively for ARM64 from this repository.
- `gridex_rockpie_tests` and `gridex_edge_tests` both passed on the board.
- A dedicated `gridex` service account, protected directories, systemd unit
  and a commissioning environment file were installed.
- The RS485/Ethernet ESP32-EVB firmware was flashed successfully over USB.
  The ESP32 received a DHCP lease on the management bench LAN and was locally
  provisioned to allow only the ROCK Pi as its Modbus TCP source.
- ROCK Pi completed read-only Modbus TCP identity and telemetry reads from the
  ESP32 canonical endpoint. The node correctly reports an unconfigured driver,
  so no RS485 inverter command can be sent.
- The identity and telemetry-range checks returned successfully from the
  canonical node map. The telemetry values were the expected zero values for
  an unconfigured node; no downstream RS485 transaction was attempted.
- The Suntech cumulative-energy implementation has been updated and code-tested
  to read registers 122–125 together in one `0x04` request. It is not yet a
  physical Suntech-cabinet verification.
- The systemd unit is **enabled** and **active**. Its commissioning lock and
  every `GRIDEX_APPROVE_*` write gate remain at `0`; the service does not
  issue BESS/PCS writes.
- The commissioning configuration binds northbound Modbus to localhost only.
  The temporary management-LAN ESP32 endpoint is read-only pilot evidence, not
  the production OT topology.
- A ROCK Pi initiated ESP32 OTA pilot passed: the image digest and per-node
  verifier were accepted, the node restarted, and its Ethernet/Modbus TCP path
  recovered. The OTA endpoint is local-only and is not reachable directly from
  WireGuard or the public Internet.

### Current safe state

No BESS connection is configured and no physical command path is enabled. The
active GrideX service remains commissioning-locked, while the ESP32 is polled
read-only through the temporary management bench LAN. The deployed binary and
configuration are ready for controlled commissioning only.

### Next stages

1. Bench-connect the known ESP32-EVB through the working management Ethernet
   only for read-only telemetry discovery. Do not connect a BESS or send a
   command in this temporary topology.
2. Record the ESP32 identity, firmware build, approved telemetry topic layout,
   TLS trust material location and its canonical Modbus map. Secrets stay out
   of Git and firmware source.
3. Configure the physical OT Ethernet with a site-specific static subnet and
   no default gateway; connect the industrial OT switch, BESS and nodes only
   after a firewall review.
4. Replace commissioning placeholders with approved site configuration, bind
   northbound Modbus only to the management address and restrict it to the
   backend source through the Site Router firewall.
5. Approve the Site Router ACL and release-secret ownership before deploying
   OTA outside the bench. The router may reach ROCK Pi, never ESP32 directly.
6. Perform read-only Suntech and node checks, then follow
   `COMMISSIONING.md` before any write approval.

## Български

Този запис описва само първия физически пилот с ROCK Pi E. В него няма
клиентски адреси, пароли, VPN данни или production endpoint-и.

### Изпълнено

- Minimal CLI образът е проверен с hash, записан на removable storage и
  платката е стартирала успешно.
- Двата Ethernet контролера са налични. Management интерфейсът има DHCP
  връзка през Site Router; OT интерфейсът е наличен, но няма carrier и няма
  настроен маршрут.
- На платката са инсталирани CMake и C++20 toolchain.
- `gridex_rockpie_service` е компилирана нативно за ARM64 от това repository.
- `gridex_rockpie_tests` и `gridex_edge_tests` са минали успешно на платката.
- Инсталирани са отделен потребител `gridex`, защитени директории, systemd
  unit и commissioning environment файл.
- RS485/Ethernet ESP32-EVB firmware е flash-нат успешно през USB. ESP32 получи
  DHCP адрес в management bench LAN и е local provision-нат да допуска само
  ROCK Pi като Modbus TCP source.
- ROCK Pi извърши read-only Modbus TCP identity и telemetry четене от ESP32
  canonical endpoint-а. Нодът коректно показва unconfigured driver, така че
  RS485 команда към инвертора не може да бъде изпратена.
- Identity и telemetry-range проверките са отговорили успешно от canonical
  node картата. Telemetry стойностите са очакваните нули за unconfigured node;
  не е направена downstream RS485 транзакция.
- Suntech cumulative-energy имплементацията е обновена и code-tested да чете
  регистри 122–125 заедно с една `0x04` заявка. Това все още не е физическа
  проверка към Suntech кабинет.
- systemd услугата е **enabled** и **active**. Commissioning lock и всеки
  `GRIDEX_APPROVE_*` write gate остават на `0`; услугата не изпраща BESS/PCS
  writes.
- Commissioning конфигурацията слуша northbound Modbus само на localhost.
  Временният ESP32 endpoint през management-LAN е read-only pilot доказателство,
  а не production OT топология.
- ROCK Pi стартира ESP32 OTA pilot успешно: image digest и verifier-ът за
  отделния нод бяха приети, нодът се рестартира, а Ethernet/Modbus TCP пътят
  се възстанови. OTA endpoint-ът е само локален и не е директно достъпен от
  WireGuard или публичния Интернет.

### Текущо безопасно състояние

Няма конфигурирана BESS връзка и няма разрешен физически command path.
Активната GrideX услуга остава commissioning-locked, а ESP32 се poll-ва
read-only през временния management bench LAN. Бинарният файл и конфигурацията
са готови само за контролирано commissioning.

### Следващи етапи

1. Познатият ESP32-EVB се свързва временно през работещия management Ethernet
   само за read-only telemetry discovery. В тази временна топология не се
   свързва BESS и не се изпраща команда.
2. Описват се ESP32 identity, firmware build, одобрената telemetry topic
   структура, местоположението на TLS trust material и canonical Modbus map.
   Тайни не се записват в Git или firmware source.
3. Настройва се физическият OT Ethernet със site-specific static subnet и без
   default gateway. Индустриалният OT switch, BESS и нодовете се включват след
   firewall review.
4. Commissioning placeholders се заменят само с одобрена site конфигурация,
   northbound Modbus се свързва само с management адреса, а Site Router
   firewall допуска единствено backend source.
5. Одобряват се Site Router ACL и собственикът на release secret, преди OTA
   извън bench. Router-ът може да достига ROCK Pi, никога ESP32 директно.
6. Правят се read-only проверки към Suntech и нодовете, след което се следва
   `COMMISSIONING.md` преди write approval.
