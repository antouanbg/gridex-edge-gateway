# Services and external interfaces

## English

The table distinguishes code that exists from infrastructure that must be
provided by a site or central backend. “Expected” is not an implementation or
an authorization to connect to live equipment.

| Component | Status | Communication | Expects / owns |
|---|---|---|---|
| `gridex_rockpie_service` | Continuous node polling and private MQTT health/telemetry implemented; physical commissioning pending | Modbus TCP client to BESS/nodes; normalized Modbus TCP listener northbound; MQTT over TLS outbound | Polls each canonical ESP32 map independently, exposes the normalized listener, and publishes health/node telemetry only after TLS broker provisioning. It has no MQTT command subscription. |
| Suntech STE-261L driver | Code and manufacturer map available; site commissioning pending | Modbus TCP, vendor port 3200, unit ID 1 | A reachable OT endpoint and readback agreement with the local HMI. Writes remain blocked until every commissioning approval is explicitly set. |
| ESP32-EVB node firmware | Flashed and polled read-only; driver unconfigured | CAN or isolated RS485 downstream; Ethernet Modbus TCP `1502`; direct MQTT disabled | One compiled driver for one device type/brand/model/revision, a known ROCK Pi source for control, TTL/sequence handling. Broker credentials belong only on ROCK Pi; the current node rejects all device commands. |
| Private MQTT broker | Backend infrastructure required | MQTT over TLS through the Site Router VPN only | Server certificate/CA, ROCK Pi publisher identity and per-site/topic ACLs. It receives telemetry; it is never a public command path. |
| GrideX backend ingestion API | Backend implementation/configuration required | Subscribes to private MQTT; calls OpenRemote APIs internally | Validates tenant/site/device identity, persists telemetry/audit data and forwards the approved normalized state. |
| OpenRemote | Backend infrastructure required | Consumes GrideX backend data; northbound Modbus control via protected Site Router path | Assets, attributes, roles, rules and agent configuration. It must not be exposed directly to browsers or field-device networks. |
| Site Router | Site infrastructure required | WireGuard peer, firewall and routing | Sole WireGuard endpoint; allows only least-privilege backend-to-management access and blocks OT-to-WAN forwarding and site-to-site routing. |
| Browser frontend | Separate frontend repository | HTTPS to GrideX backend API only | Stable GrideX DTOs and user authorization; never direct OpenRemote, MQTT, ROCK Pi or device access. |

### Pilot network rule

For the next ESP32 bench step, the management Ethernet may carry temporary
read-only node discovery. This is not the final OT design: it must not expose
the node, BESS, Modbus `3200` or northbound `1502` outside the bench scope.

## Български

Таблицата отличава готов код от инфраструктура, която се предоставя от обекта
или централния backend. „Очаква“ не означава имплементация или разрешение за
свързване към живо оборудване.

| Компонент | Статус | Комуникация | Какво очаква / за какво отговаря |
|---|---|---|---|
| `gridex_rockpie_service` | Постоянният node polling и private MQTT health/telemetry са имплементирани; physical commissioning предстои | Modbus TCP client към BESS/нодове; нормализиран northbound Modbus TCP listener; MQTT over TLS outbound | Poll-ва всяка canonical ESP32 карта независимо, предоставя нормализиран listener и публикува health/node telemetry само след TLS broker provisioning. Няма MQTT command subscription. |
| Suntech STE-261L driver | Кодът и manufacturer картата са налични; чака site commissioning | Modbus TCP, vendor port 3200, unit ID 1 | Достъпен OT endpoint и съвпадение на readback с local HMI. Write остават блокирани до изрично потвърждение на всички commissioning разрешения. |
| ESP32-EVB node firmware | Flash-нат и проверен с read-only polling; driver-ът е unconfigured | CAN или изолиран RS485 downstream; Ethernet Modbus TCP `1502`; direct MQTT disabled | Един компилиран driver за един device type/brand/model/revision, известен ROCK Pi source за control, TTL/sequence обработка. Broker credentials са само на ROCK Pi; текущият нод отказва всички команди към устройства. |
| Private MQTT broker | Нужна backend инфраструктура | MQTT over TLS само през Site Router VPN | Server certificate/CA, ROCK Pi publisher identity и ACL за обект/topic. Получава telemetry; никога не е public command path. |
| GrideX backend ingestion API | Нужна backend имплементация/конфигурация | Subscribe към private MQTT; вътрешно извиква OpenRemote APIs | Валидира tenant/site/device identity, съхранява telemetry/audit и препраща одобреното нормализирано състояние. |
| OpenRemote | Нужна backend инфраструктура | Консумира GrideX backend данни; northbound Modbus control по защитения Site Router път | Assets, attributes, roles, rules и agent конфигурация. Не се публикува директно към browser-и или field-device мрежи. |
| Site Router | Нужна site инфраструктура | WireGuard peer, firewall и routing | Единственият WireGuard endpoint; допуска least-privilege backend-to-management достъп, блокира OT-to-WAN forwarding и site-to-site routing. |
| Browser frontend | Отделно frontend repository | HTTPS само към GrideX backend API | Стабилни GrideX DTOs и user authorization; никога директен достъп до OpenRemote, MQTT, ROCK Pi или устройства. |

### Мрежово правило за пилота

В следващата ESP32 bench стъпка management Ethernet може временно да носи
read-only node discovery. Това не е финалният OT дизайн: не трябва да
публикува нода, BESS, Modbus `3200` или northbound `1502` извън bench обхвата.
