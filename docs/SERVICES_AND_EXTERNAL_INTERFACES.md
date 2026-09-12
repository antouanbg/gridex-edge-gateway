# Services and external interfaces

## English

The table distinguishes code that exists from infrastructure that must be
provided by a site or central backend. “Expected” is not an implementation or
an authorization to connect to live equipment.

| Component | Status | Communication | Expects / owns |
|---|---|---|---|
| `gridex_rockpie_service` | Built and tested on the pilot; disabled | Modbus TCP client to BESS and nodes; normalized Modbus TCP server northbound | Reads Suntech on the OT interface, polls ESP32 canonical maps, applies the safety envelope and exposes a localhost-only commissioning map. It does **not** publish MQTT today. |
| Suntech STE-261L driver | Code and manufacturer map available; site commissioning pending | Modbus TCP, vendor port 3200, unit ID 1 | A reachable OT endpoint and readback agreement with the local HMI. Writes remain blocked until every commissioning approval is explicitly set. |
| ESP32-EVB node firmware | Source available; not flashed/configured in this pilot | CAN or isolated RS485 downstream; Ethernet Modbus TCP `1502`; MQTT/TLS telemetry | One compiled driver for one device type/brand/model/revision, a known ROCK Pi source for control, TTL/sequence enforcement, and private-broker TLS provisioning. |
| Private MQTT broker | Backend infrastructure required | MQTT over TLS through the Site Router VPN only | Server certificate/CA, per-node identity and ACLs. It receives telemetry; it is never a public command path. |
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
| `gridex_rockpie_service` | Компилирана и тествана на пилота; disabled | Modbus TCP client към BESS и нодове; нормализиран northbound Modbus TCP server | Чете Suntech по OT, poll-ва canonical картите на ESP32, прилага safety envelope и предоставя localhost-only commissioning карта. В момента **не** публикува MQTT. |
| Suntech STE-261L driver | Кодът и manufacturer картата са налични; чака site commissioning | Modbus TCP, vendor port 3200, unit ID 1 | Достъпен OT endpoint и съвпадение на readback с local HMI. Write остават блокирани до изрично потвърждение на всички commissioning разрешения. |
| ESP32-EVB node firmware | Source е наличен; не е flash-нат/конфигуриран за този пилот | CAN или изолиран RS485 downstream; Ethernet Modbus TCP `1502`; MQTT/TLS telemetry | Един компилиран driver за един device type/brand/model/revision, известен ROCK Pi source за control, TTL/sequence enforcement и private-broker TLS provisioning. |
| Private MQTT broker | Нужна backend инфраструктура | MQTT over TLS само през Site Router VPN | Server certificate/CA, per-node identity и ACL. Получава telemetry; никога не е public command path. |
| GrideX backend ingestion API | Нужна backend имплементация/конфигурация | Subscribe към private MQTT; вътрешно извиква OpenRemote APIs | Валидира tenant/site/device identity, съхранява telemetry/audit и препраща одобреното нормализирано състояние. |
| OpenRemote | Нужна backend инфраструктура | Консумира GrideX backend данни; northbound Modbus control по защитения Site Router път | Assets, attributes, roles, rules и agent конфигурация. Не се публикува директно към browser-и или field-device мрежи. |
| Site Router | Нужна site инфраструктура | WireGuard peer, firewall и routing | Единственият WireGuard endpoint; допуска least-privilege backend-to-management достъп, блокира OT-to-WAN forwarding и site-to-site routing. |
| Browser frontend | Отделно frontend repository | HTTPS само към GrideX backend API | Стабилни GrideX DTOs и user authorization; никога директен достъп до OpenRemote, MQTT, ROCK Pi или устройства. |

### Мрежово правило за пилота

В следващата ESP32 bench стъпка management Ethernet може временно да носи
read-only node discovery. Това не е финалният OT дизайн: не трябва да
публикува нода, BESS, Modbus `3200` или northbound `1502` извън bench обхвата.
