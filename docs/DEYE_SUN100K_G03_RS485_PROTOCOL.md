# Deye SUN-100K-G03 RS-485 profile / Deye SUN-100K-G03 RS-485 профил

## English

This is the ESP32-EVB **read-only telemetry** profile for the Deye
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

Serial defaults are Modbus RTU, unit ID `1`, `9600 8N1`, direct UART pins
RX `GPIO36`, TX `GPIO4`, direction `GPIO13`. They are deployment/board values,
not a claim about every installed Deye unit.

`applyPowerCommand()` deliberately returns **false**. The public V118 telemetry
profile does not establish a safe external active-power-limit write contract
for this exact installed firmware. No Deye write is enabled until the on-site
manual, unit ID, serial settings, register address, scale, sign and read-back
test are approved.

### Control research boundary (2026-09-11)

The `deye_string` profile exposes an on/off control at `0x002B`, but does not
define an active-power limit for this G03 string-inverter family. Community
references to registers `244/245` concern Deye hybrid energy-management
profiles and define a maximum export limit of only `8000 W`; they must **not**
be applied to a 100 kW SUN-100K-G03. Deye's G03 manual describes zero-export
operation through an energy meter/SUN-Limiter rather than confirming a remote
Modbus active-power-limit write. The driver therefore supports telemetry only.

## Български

Това е **read-only telemetry** профилът за Deye `SUN-100K-G03` за ESP32-EVB.
Компилира се с горната PlatformIO команда. Пътят е ROCK Pi E → вътрешна OT
Ethernet/Modbus TCP мрежа → ESP32‑EVB → локален RS‑485/Modbus RTU → Deye.

Профилът чете типа, номиналната мощност, състоянието, произведената енергия,
изходната мощност и алармите от посочените регистри. Приемането на устройство
изисква номиналната мощност да е между 90 и 110 kW. Комуникационните стойности
по подразбиране са unit ID 1 и 9600 8N1, но се валидират на конкретния обект.

Командите за мощност са заключени. Публичната карта потвърждава телеметрията,
но не и безопасен write регистър за ограничение на активната мощност на този
firmware. Преди разрешаване на какъвто и да е запис са нужни документ от Deye,
проверка на адрес/мащаб/знак и read-back тест на реалния инвертор.

### Граница на проучването за управление (2026-09-11)

Профилът `deye_string` има on/off при `0x002B`, но няма потвърдена команда за
ограничение на активната мощност за G03 string семейството. Обсъжданите във
форуми `244/245` са за Deye hybrid режими и са с лимит само `8000 W`; те **не
трябва** да се използват за 100 kW SUN-100K-G03. Ръководството на Deye описва
zero-export чрез електромер/SUN-Limiter, а не потвърден remote Modbus write.
Затова драйверът остава само за телеметрия.
