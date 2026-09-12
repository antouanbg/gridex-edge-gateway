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
- The systemd unit is deliberately **disabled** and **inactive**.
- The commissioning configuration binds northbound Modbus to localhost only,
  defines no BESS/node endpoint and keeps every `GRIDEX_APPROVE_*` write gate
  at `0`.

### Current safe state

No GrideX process is listening on the LAN, no BESS connection is configured,
no ESP32 node is polled and no physical command path is enabled. The deployed
binary and configuration are ready for controlled commissioning only.

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
5. Perform read-only Suntech and node checks, then follow
   `COMMISSIONING.md` before any write approval or service enablement.

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
- systemd услугата е умишлено **disabled** и **inactive**.
- Commissioning конфигурацията слуша northbound Modbus само на localhost,
  няма BESS/node endpoint и държи всеки `GRIDEX_APPROVE_*` write gate на `0`.

### Текущо безопасно състояние

Няма GrideX процес, който слуша на LAN, няма конфигурирана BESS връзка, няма
polling на ESP32 нод и няма разрешен физически command path. Бинарният файл и
конфигурацията са готови само за контролирано commissioning.

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
5. Правят се read-only проверки към Suntech и нодовете, след което се следва
   `COMMISSIONING.md` преди write approval или enable на услугата.
