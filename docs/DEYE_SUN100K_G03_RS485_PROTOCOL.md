# Deye SUN-100K-G03 RS-485 profile / Deye SUN-100K-G03 RS-485 профил

## English

This is the ESP32-EVB telemetry and active-power-limiter profile for the Deye
`SUN-100K-G03` three-phase string inverter. It is compiled with:

```bash
pio run -e esp32-evb-deye-sun100k-g03-rs485
```

Path: `ROCK Pi E -- OT Ethernet / Modbus TCP 1502 --> ESP32-EVB -- local
RS-485 / Modbus RTU --> Deye SUN-100K-G03`.

The product family is confirmed as a 70–110 kW, six-MPPT string inverter with
RS-485/RS-232 communication by [Deye](https://www.deyeinverter.com/product/three-phase-string-inverter/sun70-75-80-90-100-110kg03-70110kw-three-phase-6-mppt.html).
The initial register profile is derived from the open
[ha-solarman `deye_string` V118 profile](https://github.com/davidrapan/ha-solarman/blob/main/custom_components/solarman/inverter_definitions/deye_string.yaml),
which lists the G03 family including the 100 kW model.

| Field | Modbus holding register(s) | Decode |
|---|---:|---|
| Device type | `0x0000` | expects string-inverter code `0x0002` |
| Rated power | `0x0010–0x0011` | UInt32, 0.1 W; accepted only 90–110 kW |
| State | `0x003B` | UInt16 |
| Total production | `0x003F–0x0040` | UInt32, 0.1 kWh |
| Output power | `0x0056–0x0057` | UInt32, 0.1 W |
| Alarm word | `0x0065–0x0066` | UInt32; compacted to the MBUS alarm field |
| External-control enable | `76` (`0x004C`) | Optional UInt16: `0` disable, `1` enable |
| Active-power regulation | `77` (`0x004D`) | R/W UInt16, `0…1000`, scale `0.1%` |

Serial defaults are Modbus RTU, unit ID `1`, `9600 8N1`, direct UART pins
RX `GPIO36`, TX `GPIO4`, direction `GPIO13`. They are deployment/board values,
not a claim about every installed Deye unit.

The supplied official Deye `Modbus RTU Protocol V1.31 / V1.32 for String PV
Inverters` confirms Modbus RTU `9600 8N1`, unit ID `1` by default and function
codes `0x03`, `0x06`, `0x10`. The driver uses `0x06` for the limiter and reads
register `77` with `0x03` immediately afterwards. A GrideX kW request is
converted using the actual rated power read from the inverter; for example,
50 kW on a 100 kW unit writes `500` (50.0%).

Writes remain **off by default** in deployment configuration. An administrator
must explicitly set `writes_enabled=true` after a read-only bench check. The
optional register-76 write is separately controlled by
`enable_register_76=true` because direct register-77 writes are sufficient on
most firmware versions. On Edge TTL expiry, GrideX writes a `0.0%` limiter.
The Deye device's own configured communication-timeout policy remains a
separate site safety setting.

The ESP32 provisioning namespace is `gridex-deye`. It accepts the following
non-secret values: `unit_id` (`1…247`, default `1`), `writes_enabled` (default
`false`), `enable_register_76` (default `false`) and
`maximum_limit_x10pct` (`0…1000`, default `1000`). The maximum is an Edge
ceiling and can only lower, never exceed, the inverter's 100.0% protocol
limit. Browser clients must never set these values directly.

Registers `244/245` are not part of the SUN-100K-G03 string-inverter map and
are excluded from this driver.

## Български

Това е telemetry и active-power-limiter профилът за Deye `SUN-100K-G03` за
ESP32-EVB. Компилира се с горната PlatformIO команда. Пътят е ROCK Pi E →
вътрешна OT Ethernet/Modbus TCP мрежа → ESP32‑EVB → локален RS‑485/Modbus RTU
→ Deye.

Профилът чете типа, номиналната мощност, състоянието, произведената енергия,
изходната мощност и алармите от посочените регистри. Приемането на устройство
изисква номиналната мощност да е между 90 и 110 kW. Комуникационните стойности
по подразбиране са unit ID 1 и 9600 8N1, но се валидират на конкретния обект.

Предоставеният официален `Modbus RTU Protocol V1.31 / V1.32 for String PV
Inverters` потвърждава `9600 8N1`, unit ID `1` по подразбиране и function codes
`0x03`, `0x06`, `0x10`. Драйверът записва limiter-а с `0x06` в регистър 77 и
веднага го прочита обратно с `0x03`. GrideX подава kW, които се преобразуват
спрямо реално прочетената номинална мощност: 50 kW при 100 kW инвертор записва
`500` (50.0%).

Записите са изключени по подразбиране в deployment конфигурацията. Администратор
ги включва с `writes_enabled=true` след read-only bench тест. Регистър 76 е
отделна опция `enable_register_76=true`, защото повечето firmware версии
приемат директен запис в 77. При изтичане на Edge TTL GrideX записва лимит
`0.0%`. Настройката Comm Timeout Protection в самия Deye остава отделна
site safety настройка.

ESP32 използва provisioning namespace `gridex-deye` със стойности без тайни:
`unit_id` (`1…247`, по подразбиране `1`), `writes_enabled` (по подразбиране
`false`), `enable_register_76` (по подразбиране `false`) и
`maximum_limit_x10pct` (`0…1000`, по подразбиране `1000`). Последният е Edge
таван и може само да намалява, но не и да надвишава протоколния лимит 100.0%.
Browser клиентите не трябва да записват тези стойности директно.

Регистри `244/245` не са част от string картата за SUN-100K-G03 и не се
използват от този драйвер.
