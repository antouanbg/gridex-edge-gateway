# Suntech STE-261L / BCQ protocol profile

Това е изпълнимият договор за първата GrideX интеграция. Кабинетът е all-in-one AC-coupled BESS и се свързва директно към OT Ethernet порта на ROCK Pi E чрез Modbus TCP, порт `3200`, unit ID `1`. ESP32 нод не участва в този път.

## Потвърдени правила

- Адресите се използват точно както са изписани от производителя. Не се прилага `+1` или `-1` offset.
- Каноничният знак в GrideX е положителен за разряд и отрицателен за заряд.
- Активната и реактивната мощност са signed `INT16` с коефициент `10`: `100.0 kW` се записва като `1000`.
- Команда за мощност се разрешава само когато `5003=1`, `5001=0`, `5002=1`, няма активни PCS/BMS faults и BMS лимитите са валидни.
- `5001` и `5002` се четат като предпоставки, но не се променят автоматично след startup.
- BMS `127/128` са авторитетните динамични максимуми за заряд и разряд. Всяка заявка се clamp-ва в този envelope.
- Heartbeat се активира през `5302=1`; в `5301` се задават `60 s`, а локалният Edge го обновява през `35 s` и винаги преди `50 s`.
- При изтичане на heartbeat мощността се връща към нула без shutdown на кабинета.

## Критични регистри

| Област | Адрес | Значение | Употреба |
|---|---:|---|---|
| Coil | 1 | PCS overall fault | блокира команда при `1` |
| Coil | 2 | on-grid/off-grid | изисква `0` |
| Coil | 4 | PCS communication fault | блокира команда при `1` |
| Coil | 184 | BMS fault | блокира команда при `1` |
| Coil | 185 | BMS communication fault | блокира команда при `1` |
| Holding | 5001 | grid/off-grid mode | изисква `0`, read-only за автоматиката |
| Holding | 5002 | current/voltage source mode | изисква `1`, read-only за автоматиката |
| Holding | 5003 | power on/off | изисква `1` |
| Holding | 5005 | active power setpoint | signed kW ×10 |
| Holding | 5006 | reactive power setpoint | signed kVAr ×10 |
| Holding | 5301 | heartbeat countdown | `60 s`, refresh `35 s` |
| Holding | 5302 | heartbeat enable | `1` |
| Input | 9 | actual active power | signed kW ÷10 |
| Input | 16 | PCS state | 0 off, 1 standby, 2 charge, 3 discharge |
| Input | 101–105 | BMS current/SOC/SOH/voltage/state | основна телеметрия |
| Input | 126 | BMS system flags | bits 0/1 забраняват charge/discharge |
| Input | 127/128 | max charge/discharge | динамични BMS лимити, kW ÷10 |
| Input | 129/130 | daily charge/discharge energy | kWh ÷10 |

## Polling групи

- `Fast control`, 1 s: PCS power/status, BMS SOC/current, registers 126–128, command prerequisites и fault coils.
- `Operational`, 5 s: напрежения, температурни крайности, SOH, daily energy, cooling state и safety I/O.
- `Diagnostic`, 30–60 s: fault words, 260 cell voltages, 100 cell temperatures и accumulated energy counters.

Клетъчните стойности и охлаждането са за диагностика и predictive maintenance; те не трябва да забавят fast control цикъла.

## Разделение на отговорностите

OpenRemote и оптимизаторът изчисляват желана мощност от IBEX, прогнози, графици, небаланс и ERP товар. Edge Gateway валидира свежестта на командата, software fuse, BMS envelope, режима на PCS и комуникационното състояние. Само Edge пише към `5005/5006` и поддържа heartbeat.
