# ROCK Pi ↔ ESP32 MQTT bridge / ROCK Pi ↔ ESP32 MQTT мост

## English

ESP32-EVB nodes do not connect to MQTT in the default firmware. They expose
their canonical MBUS v4 map through Modbus TCP `1502` on the isolated OT
Ethernet network. ROCK Pi polls each node and is the sole MQTT client crossing
the Site Router VPN.

```text
Sensor / RS-485 / CAN device
  -> ESP32 driver -> OT Modbus TCP 1502 -> ROCK Pi polling service
  -> private MQTT broker through Site Router VPN -> backend ingestion

Backend command -> private MQTT broker -> ROCK Pi command bridge
  -> OT Modbus TCP 1502 -> ESP32 TTL/sequence gate -> device driver
```

No device command goes directly from MQTT to an ESP32. ROCK Pi validates the
topic, bounded payload, slot, sequence, requested power and 1–30 second TTL.
The ESP32 validates the source IP, Modbus command window, monotonic sequence
and TTL again. A driver may still reject the command; Deye SUN-100K-G03 does
so until its write map is validated.

### MQTT contract

Topic prefix is deployment configuration (`GRIDEX_MQTT_TOPIC_PREFIX`), normally
`gridex/v1`. `<site>` and `<gateway>` are deployment identifiers, never source
code literals.

| Direction | Topic | QoS | Payload |
|---|---|---:|---|
| ROCK Pi → backend | `.../sites/<site>/edge/<gateway>/nodes/<slot>/telemetry` | 1 | `schemaVersion`, `observedAt`, `slot`, `driverId`, `nodeType`, `online`, `quality`, `actualPowerKw`, `energyWh`, `deviceState`, `alarmBits`, `heartbeat` |
| backend → ROCK Pi | `.../sites/<site>/edge/<gateway>/nodes/<slot>/command` | 1 | `sequence`, `requestedPowerKw`, `enabled`, `ttlSeconds` |

The Edge bridge accepts payloads up to 1024 bytes, 1–30 seconds TTL, a
non-zero UInt16 sequence and a power range of −3276.8…3276.7 kW. It keeps at
most 16 pending commands. The backend must authorize the user and target Asset
before publishing; it must persist command/audit result and consume telemetry.

## Български

По подразбиране ESP32-EVB няма MQTT клиент. Нодът предоставя каноничната MBUS
v4 карта през Modbus TCP `1502` по изолираната OT Ethernet мрежа. ROCK Pi я
чете и е единственият MQTT клиент през VPN тунела на Site Router.

Пътят за телеметрия е: сензор или устройство → ESP32 драйвер → Modbus TCP към
ROCK Pi → private MQTT → backend. Пътят за команда е обратният, но винаги
минава през ROCK Pi и след това през допълнителните ESP32 проверки за source,
sequence и TTL. MQTT никога не подава команда директно към ESP32.

Темите и полетата са посочени в таблицата по-горе. Backend-ът проверява
правата, записва audit резултат и приема телеметрията; Edge ограничава всички
стойности и конкретният драйвер има право да откаже команда.
