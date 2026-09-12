# Live hardware status / Текущ хардуерен статус

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

This is a reconciled, read-only pilot record from 2026-09-12. It preserves
verified observations from the earlier live-status and commissioning branches,
while replacing superseded claims. It is not continuous monitoring and contains
no addresses, identifiers, credentials or production endpoints.

| Component | Verified pilot state |
|---|---|
| ROCK Pi operating system | Network, time synchronization, logging and SSH were healthy during the inspection; no failed systemd units were observed. |
| `gridex-rockpie.service` | Enabled and active with commissioning lock and every `GRIDEX_APPROVE_*` gate at `0`. No BESS/PCS write is permitted. |
| Northbound listener | Loopback-only, read-only pilot configuration. It is not published to the site LAN, VPN or Internet. |
| Management Ethernet | Used only for the temporary local bench connection. |
| OT Ethernet | Present but not commissioned for production; it has no approved OT route, carrier, address or firewall policy. |
| ESP32 Ethernet / Modbus TCP | ROCK Pi completed repeated read-only identity and telemetry reads from the canonical map (map version 4). The loop heartbeat advanced across samples. Map version 5 provisioning/health/watchdog firmware is built and tested but has not yet been flashed to this pilot. |
| Local telemetry journal | Code-tested only. It has not yet been installed or observed on the physical ROCK Pi; it records local snapshots and state transitions without control payloads. |
| ESP32 field driver | `UnconfiguredDriver` / DriverMissing evidence; sampled power and energy are not inverter measurements, and no downstream RS485/CAN transaction is approved. |
| Recovery | ESP32 reset, Ethernet disconnect/reconnect and controlled ROCK Pi reboot returned the normalized node state from offline to online while the locked service stayed active. |
| OTA update service | ROCK Pi initiated a verified local ESP32 firmware update. The node verified the per-node token verifier and image SHA-256, rebooted, and Ethernet/Modbus TCP recovered. |
| MQTT / backend | No private broker CA/client identity has been provisioned. Live MQTT, backend ingestion and OpenRemote mapping are not commissioned. |

The resource measurements captured during the earlier inspection were healthy
samples, not acceptance limits. An ESP32 USB serial session may reset the board;
do not treat opening a serial port as uptime-preserving. The exact ESP32 build
revision is not exposed by the canonical status map and must be recorded in the
future release identity process.

## Български

Това е съгласуван read-only pilot запис от 2026-09-12. Той запазва
потвърдените наблюдения от по-ранните branches за live status и commissioning,
като заменя отменените твърдения. Не е непрекъснат мониторинг и не съдържа
адреси, идентификатори, credentials или production endpoint-и.

| Компонент | Потвърдено pilot състояние |
|---|---|
| ROCK Pi операционна система | По време на проверката network, time synchronization, logging и SSH бяха изправни; не са наблюдавани failed systemd units. |
| `gridex-rockpie.service` | Enabled и active с commissioning lock и всеки `GRIDEX_APPROVE_*` gate на `0`. BESS/PCS write не е разрешен. |
| Northbound listener | Само на loopback в read-only pilot конфигурация. Не е публикуван към site LAN, VPN или Интернет. |
| Management Ethernet | Използва се само за временната локална bench връзка. |
| OT Ethernet | Наличен, но не е production-commissioned; няма одобрен OT route, carrier, адрес или firewall policy. |
| ESP32 Ethernet / Modbus TCP | ROCK Pi направи повторяеми read-only identity и telemetry reads от canonical картата (map version 4). Loop heartbeat-ът се увеличаваше между пробите. Firmware-ът за map version 5 provisioning/health/watchdog е build-нат и тестван, но още не е flash-нат на този пилот. |
| Local telemetry journal | Само code-tested. Още не е инсталиран или наблюдаван на физическия ROCK Pi; записва локални snapshots и state transitions без control payload-и. |
| ESP32 field driver | Има доказателство за `UnconfiguredDriver` / DriverMissing; sample power и energy не са inverter measurements и не е одобрена downstream RS485/CAN транзакция. |
| Recovery | Reset на ESP32, Ethernet disconnect/reconnect и контролиран ROCK Pi reboot върнаха нормализирания node state offline → online, докато заключената услуга остана active. |
| OTA услуга | ROCK Pi стартира потвърдено локално ESP32 firmware обновяване. Нодът провери verifier-а за token на отделния нод и SHA-256 на образа, рестартира се и Ethernet/Modbus TCP се възстанови. |
| MQTT / backend | Няма provision-нати private broker CA/client identity. Live MQTT, backend ingestion и OpenRemote mapping не са commission-нати. |

Ресурсните измервания от по-ранната проверка са изправни проби, а не
acceptance лимити. USB serial сесия към ESP32 може да рестартира платката; не
приемай отварянето на serial port за запазващо uptime. Точната ESP32 build
ревизия не се показва от canonical status картата и трябва да се запише в
бъдещия release identity процес.
