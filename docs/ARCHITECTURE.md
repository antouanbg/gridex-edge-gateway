# Edge architecture V2

## English

```text
GrideX backend / OpenRemote
       |                                     ^
       | CONTROL over Site Router WireGuard  | private MQTT telemetry
       v                                     |
ROCK Pi E northbound Modbus TCP :1502 --------+
       |
       +-- local strategy arbiter + software fuse + BMS envelope
       |
       +-- direct Modbus TCP :3200 -> Suntech STE-261L
       |
       +-- isolated OT Ethernet -> ESP32-EVB node :1502
                                    |
                                    +-- onboard CAN -> one device
                                    \-- isolated UEXT RS485 -> one device

ESP32-EVB telemetry -> OT Ethernet / Modbus TCP :1502 -> ROCK Pi E
                    -> Site Router WireGuard -> private MQTT broker
                    -> backend ingestion -> PostgreSQL / OpenRemote Assets
```

The browser reaches only the GrideX API, never OpenRemote or Edge directly.
ROCK Pi E and ESP32 do not run WireGuard; the Site Router owns the site's
unique peer. CONTROL and TELEMETRY are separated, there is no site-to-site
routing, and the OT/BESS network has no direct backend route.

The ROCK Pi E polls every node's canonical map over Ethernet. Vendor mapping
stays in the compiled node driver. Cloud MQTT is telemetry-only. Device
commands use the deterministic ROCK Pi E path, a per-node sequence, a short TTL
and zero-power fallback.

## Български

```text
GrideX backend / OpenRemote
       | CONTROL през WireGuard на Site Router
       v
ROCK Pi E northbound Modbus TCP :1502
       |--> OT Ethernet / Modbus TCP :1502 --> ESP32-EVB --> CAN / RS485 устройство
       |--> OT Ethernet / Modbus TCP :3200 --> Suntech STE-261L

ESP32-EVB -> OT Modbus TCP -> ROCK Pi E -> private MQTT през Site Router
          -> backend ingestion -> PostgreSQL / OpenRemote Assets
```

ROCK Pi E получава команди от OpenRemote през WireGuard тунела на Site Router,
прилага локалния безопасен работен диапазон и ги изпраща по изолираната OT
Ethernet мрежа към конкретния ESP32-EVB. Нодът превежда към вградения CAN или
към външен изолиран RS485 трансивър.

ROCK Pi E публикува телеметрията на нода към private MQTT през VPN и приема
валидирани broker команди, които препраща по OT Modbus TCP. Backend ingestion
услугата пази телеметрията и синхронизира нужните OpenRemote Assets. ESP32 няма
MQTT команда или WireGuard. Няма site-to-site routing или директен route от
backend към OT/BESS мрежата.

Всеки нод има един компилиран driver_id за един тип, бранд, модел и ревизия.
Командата е валидна само при правилен source, sequence и TTL; след timeout
изходът става 0 kW.
