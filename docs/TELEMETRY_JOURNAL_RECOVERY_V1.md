# Telemetry journal recovery contract v1 / Договор v1 за възстановяване на telemetry journal

**Status / Статус:** Draft for owner approval. This document defines a future
data-recovery capability only. It neither enables it nor permits device control.

## English

### Scope and safety

The existing ROCK Pi journal remains local, bounded and observation-only. The
future recovery path is **at-least-once delivery with backend deduplication**:
an interrupted delivery may repeat a record, but the backend accepts it once.
It must use only the Site Router VPN and private, mutually authenticated MQTT.
It must not expose the ROCK Pi filesystem, add a public listener, route OT/BESS
to the backend, unlock writes or carry an inverter/BESS command.

### Roles

| Component | Future responsibility | Explicitly excluded |
|---|---|---|
| ROCK Pi journal | Append bounded normalized observations locally. | Export, acknowledgement and replay in the current release. |
| Edge exporter | Read eligible records, publish outbound and retain its delivery checkpoint. | Device commands, driver changes and inbound control. |
| Private MQTT broker | Transport authenticated record and acknowledgement messages. | Public listener and command channel. |
| Backend recovery worker | Authenticate, validate, deduplicate and persist records before acknowledgement. | Direct OT access and device control. |
| OpenRemote | Receive approved current summaries only. | Raw journal storage and acknowledgement authority. |

### Required v1 records

The current journal has only a process-local `sequence`; it is **not** export
compatible. A future Edge migration must append a durable UUID `recordId` at
creation. It must never use a node address, credential, VPN value or a control
payload as record data.

```json
{
  "schemaVersion": 1,
  "recordId": "uuid",
  "siteId": "logical-site-id",
  "gatewayId": "logical-gateway-id",
  "observedAt": "RFC3339 UTC timestamp",
  "kind": "snapshot | transition",
  "slot": 1,
  "pollStatus": "online | transport_failure | identity_failure | telemetry_failure",
  "previousPollStatus": "optional status",
  "telemetry": { "online": true, "consecutiveFailures": 0, "values": "normalized safe values" }
}
```

The private MQTT topic names are logical templates, not deployment values:

```text
gridex/v1/journal/records
gridex/v1/journal/acks
```

Tenant/site authorization comes from the mutually authenticated client identity
and backend policy, not an untrusted topic field. An acknowledgement contains
only `schemaVersion`, `recordId`, `status` and `acceptedAt`. `accepted` and
`duplicate` advance the local delivery checkpoint; `rejected` keeps the record
and creates an operator-visible ingestion error. The acknowledgement channel is
not a control channel and cannot alter a driver, relay, power setpoint or
commissioning gate.

### Persistence and recovery rules

The backend stores every accepted record in GrideX PostgreSQL/Timescale with
`record_id` as the unique deduplication key, plus logical site/gateway IDs,
observation time, ingestion time, kind, normalized payload and audit outcome.
It commits this transaction before publishing an acknowledgement. OpenRemote
receives only approved latest health/summary attributes after persistence; raw
journal retention remains in PostgreSQL.

The Edge retries unacknowledged records with bounded exponential backoff. It
does not delete local journal files on an acknowledgement; existing bounded
rotation remains the offline-retention mechanism. The first implementation must
define behaviour when retention rotation occurs before acknowledgement and must
surface that loss as an audit event.

### Approval and acceptance criteria

Before implementation, approve this contract and the identity/retention policy.
The coordinated Edge and Backend Pull Requests must prove:

1. duplicate, reordered, broker-outage, restart and acknowledgement-loss cases;
2. no duplicate PostgreSQL records for one `recordId`;
3. no export of endpoints, secrets, VPN data or control payloads;
4. no MQTT command subscription or device write; and
5. a valid OpenRemote summary only after durable backend persistence.

## Български

### Обхват и безопасност

Съществуващият журнал на ROCK Pi остава локален, ограничен по размер и само за
наблюдение. Бъдещият recovery път е **at-least-once доставка с backend
deduplication**: прекъсната доставка може да повтори запис, но backend-ът го
приема само веднъж. Ползват се единствено Site Router VPN и private,
взаимно удостоверен MQTT. Не се излага файловата система на ROCK Pi, не се
добавя публичен listener, не се route-ва OT/BESS към backend, не се отключват
write-ове и не се пренасят inverter/BESS команди.

### Роли

| Компонент | Бъдеща отговорност | Изрично изключено |
|---|---|---|
| ROCK Pi journal | Локално добавяне на ограничени нормализирани наблюдения. | Export, acknowledgement и replay в текущия release. |
| Edge exporter | Чете допустими записи, публикува outbound и пази delivery checkpoint. | Device команди, driver промени и inbound control. |
| Private MQTT broker | Пренася удостоверени record и acknowledgement съобщения. | Public listener и command channel. |
| Backend recovery worker | Удостоверява, валидира, deduplicate-ва и пази записи преди acknowledgement. | Директен OT достъп и device control. |
| OpenRemote | Получава само одобрени актуални summary данни. | Съхранение на raw journal и acknowledgement authority. |

### Задължителни v1 записи

Текущият журнал има само process-local `sequence`; той **не е** съвместим с
export. Бъдеща Edge миграция трябва да добавя устойчив UUID `recordId` при
създаване. Той никога не използва node адрес, credential, VPN стойност или
control payload като данни в записа.

```json
{
  "schemaVersion": 1,
  "recordId": "uuid",
  "siteId": "logical-site-id",
  "gatewayId": "logical-gateway-id",
  "observedAt": "RFC3339 UTC timestamp",
  "kind": "snapshot | transition",
  "slot": 1,
  "pollStatus": "online | transport_failure | identity_failure | telemetry_failure",
  "previousPollStatus": "optional status",
  "telemetry": { "online": true, "consecutiveFailures": 0, "values": "normalized safe values" }
}
```

Private MQTT topic имената са логически шаблони, а не deployment стойности:

```text
gridex/v1/journal/records
gridex/v1/journal/acks
```

Tenant/site authorization идва от взаимно удостоверената client identity и
backend policy, а не от недоверено topic поле. Acknowledgement съдържа само
`schemaVersion`, `recordId`, `status` и `acceptedAt`. `accepted` и `duplicate`
придвижват local delivery checkpoint; `rejected` запазва записа и създава
операторски видима ingestion грешка. Acknowledgement каналът не е control
канал и не може да променя driver, relay, power setpoint или commissioning gate.

### Правила за съхранение и възстановяване

Backend-ът пази всеки приет запис в GrideX PostgreSQL/Timescale с `record_id`
като unique deduplication ключ, плюс логически site/gateway ID, време на
наблюдение, време на приемане, kind, нормализиран payload и audit резултат.
Той commit-ва транзакцията преди да публикува acknowledgement. OpenRemote
получава само одобрени latest health/summary attributes след съхранението; raw
journal retention остава в PostgreSQL.

Edge повтаря непотвърдените записи с ограничен exponential backoff. Той не
изтрива локални journal файлове при acknowledgement; съществуващото ограничено
rotation остава механизмът за offline retention. Първата имплементация трябва
да определи поведението при rotation преди acknowledgement и да отчете тази
загуба като audit събитие.

### Одобрение и приемателни критерии

Преди имплементация се одобряват този договор и identity/retention policy.
Координираните Edge и Backend Pull Request-и трябва да докажат:

1. duplicate, reordered, broker-outage, restart и acknowledgement-loss случаи;
2. липса на duplicate PostgreSQL записи за един `recordId`;
3. липса на export на endpoints, secrets, VPN данни или control payload-и;
4. липса на MQTT command subscription или device write; и
5. валиден OpenRemote summary едва след устойчиво backend съхранение.
