# Edge architecture V2

## English

```text
OpenRemote / GrideX backend
       |
       | CONTROL over site-router WireGuard
       v
ROCK Pi E northbound Modbus TCP :1502
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
                    -> site-router WireGuard -> VPN-only MQTT -> backend
```

The browser never reaches OpenRemote or Edge directly. ROCK Pi E and ESP32 do
not run WireGuard; the site router owns the site's unique peer. CONTROL and
TELEMETRY are separated, there is no site-to-site routing, and the OT/BESS
network has no direct backend route.

The ROCK Pi E polls every node's canonical map over Ethernet and is the sole
MQTT client for node telemetry and broker commands. Vendor mapping stays in the
compiled node driver. Device commands use the deterministic ROCK Pi E path, a
per-node sequence, a short TTL and zero-power fallback.

## Български

ROCK Pi E получава команди от OpenRemote през WireGuard тунела на site router-а,
прилага локалния safety envelope и ги изпраща по изолиран OT Ethernet към
конкретния ESP32-EVB. Нодът превежда към вградения CAN или към външен изолиран
RS485 трансивър.

ROCK Pi E публикува телеметрията на нода към VPN-only MQTT и приема валидирани
broker команди, които препраща по OT Modbus TCP. ESP32 няма MQTT команда или
WireGuard. Няма site-to-site routing или директен route от backend към OT/BESS
мрежата.

Всеки нод има един компилиран driver_id за един тип, бранд, модел и ревизия.
Командата е валидна само при правилен source, sequence и TTL; след timeout
изходът става 0 kW.
