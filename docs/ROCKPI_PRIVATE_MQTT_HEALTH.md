# ROCK Pi E private MQTT health bridge / Частен MQTT health bridge на ROCK Pi E

Repository / GitHub: `antouanbg/gridex-edge-gateway`

## English

`gridex_rockpie_service` has two continuous outbound telemetry functions:

1. it polls every endpoint in `GRIDEX_NODE_ENDPOINTS` by Modbus TCP (`0x0000`
   identity and `0x0040` telemetry blocks); and
2. it publishes a retained Edge-health message and non-retained per-node
   telemetry to the private MQTT broker over TLS.

The publisher accepts **only** an `mqtts://` broker URL with a CA file. It has
no subscription, no MQTT command handler and no direct ESP32 MQTT path. The
Site Router carries the TLS session across the per-site WireGuard tunnel. This
does not create a route from the backend to OT/BESS.

### Topics and payloads

```text
gridex/v1/sites/<site-id>/edge/<gateway-id>/health        retained, QoS 1
gridex/v1/sites/<site-id>/edge/<gateway-id>/nodes/<slot>/telemetry  QoS 1
```

Health includes state, PCS-heartbeat condition, safety/control state,
northbound-listener readiness and online/total node count. Node telemetry
includes the normalized node identity/state, driver ID, quality, power, energy,
alarm bits and node heartbeat. The backend validates site/gateway identity and
maps the messages to PostgreSQL/OpenRemote; browser clients never connect to
this broker.

### Provisioning requirements

- `libmosquitto-dev` is required when compiling the ROCK Pi service;
- `GRIDEX_MQTT_CA_FILE` points to the private broker CA;
- an optional username uses `GRIDEX_MQTT_PASSWORD_FILE`, which is owned by the
  local service account and is never committed;
- client certificate and key are optional, but must be configured together;
- each Site uses a distinct client identity and a least-privilege publish ACL.

The service can poll nodes without a broker configuration. It emits no MQTT
traffic until all required MQTT settings are present and a TLS connection is
established. It remains read-only for the current pilot: all
`GRIDEX_APPROVE_*` values stay `0`.

## Български

`gridex_rockpie_service` има две постоянни outbound telemetry функции:

1. обхожда всеки endpoint от `GRIDEX_NODE_ENDPOINTS` чрез Modbus TCP
   (identity блока `0x0000` и telemetry блока `0x0040`); и
2. публикува retained Edge-health съобщение и non-retained telemetry за всеки
   нод към private MQTT broker през TLS.

Publisher-ът приема **само** `mqtts://` broker URL с CA файл. Той няма
subscription, MQTT command handler или директен MQTT път от ESP32. Site Router
пренася TLS сесията през WireGuard тунела за конкретния обект. Това не създава
route от backend към OT/BESS.

### Topics и payload-и

```text
gridex/v1/sites/<site-id>/edge/<gateway-id>/health        retained, QoS 1
gridex/v1/sites/<site-id>/edge/<gateway-id>/nodes/<slot>/telemetry  QoS 1
```

Health съобщението съдържа state, PCS-heartbeat състояние, safety/control
състояние, готовност на northbound listener-а и брой online/общ брой нодове.
Telemetry за нода съдържа нормализирани identity/state, driver ID, quality,
power, energy, alarm bits и node heartbeat. Backend-ът валидира
site/gateway identity и преобразува съобщенията за PostgreSQL/OpenRemote;
browser клиентите никога не се свързват с този broker.

### Изисквания за provisioning

- при компилиране на ROCK Pi услугата е нужен `libmosquitto-dev`;
- `GRIDEX_MQTT_CA_FILE` сочи към CA на private broker-а;
- опционалният username използва `GRIDEX_MQTT_PASSWORD_FILE`, който се държи
  от локалния service account и никога не се commit-ва;
- client certificate и key са опционални, но се задават заедно;
- всеки Обект има отделна client identity и least-privilege publish ACL.

Услугата може да poll-ва нодове без broker конфигурация. Тя не изпраща MQTT
трафик, докато не са налични всички нужни MQTT настройки и няма TLS връзка.
За текущия пилот остава само read-only: всички `GRIDEX_APPROVE_*` стойности са
`0`.
