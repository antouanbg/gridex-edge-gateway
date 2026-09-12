# ROCK Pi E ↔ ESP32-EVB bench contract

## English

This contract is for the temporary read-only bench connection through the
working management Ethernet. It is not the production OT topology.

### Direction and responsibility

```text
ESP32-EVB canonical map :1502  --read FC03-->  ROCK Pi node poller
ROCK Pi northbound map :1502   --read FC04-->  protected backend/OpenRemote agent
Planned forwarding: ROCK Pi -> Site Router VPN -> private MQTT -> backend ingestion

Suntech STE-261L :3200         --read FC01/FC03/FC04--> ROCK Pi driver
```

The ROCK Pi is the only permitted Modbus TCP client for an ESP32 node. The node
accepts its TCP connection only after local serial provisioning with:

```text
rockpi <ROCK_PI_IPV4>
status
```

This stores only the allowed source address in ESP32 NVS. It does not store a
password, enable MQTT, select a driver or permit a command. The `status`
command reports the node UID, identity, Ethernet address and configured ROCK
Pi source over the local USB serial console.

Direct ESP32 MQTT is disabled even if legacy NVS enables it. The ROCK Pi MQTT
publisher is separate pending work; the pilot verified only Modbus reads.

### State forwarding

ROCK Pi polls node holding registers `0x0000..0x000C` and `0x0040..0x0045`.
It forwards online state, identity, driver ID, quality, heartbeat, power,
energy, device state, alarms, data age and MQTT connection state into its
northbound input-register node slots beginning at `0x0100`.

The ESP32 command window remains `0x0014..0x0017`, but this bench process does
not write it. A node built with the current unconfigured driver rejects all
power commands even if a client attempted one.

### Suntech settings and state

Suntech is not routed through ESP32. The ROCK Pi reads it directly by Modbus
TCP. Its northbound map forwards, read-only: PCS on state, grid-tied state,
current-source mode, SOC upper/lower limits, BMS charge/discharge limits,
status/fault flags and telemetry. Accumulated charge/discharge energy registers
`122..125` are read as one FC04 request to preserve a common snapshot.

| Northbound input address | Read-only Suntech value |
|---:|---|
| `5`, `6` | live BMS maximum charge / discharge kW ×10 |
| `7`, `8`, `25` | BMS status, system flags and alarm bits |
| `29`, `30` | observed SOC upper / lower limits (%) |
| `32`, `33`, `34` | PCS power-on, grid-tied and current-source states |

No Suntech write, heartbeat enable, ESP32 command, MQTT secret or backend
address is part of this bench contract.

## Български

Този договор е за временната read-only bench връзка през работещия management
Ethernet. Това не е production OT топология.

### Посока и отговорност

```text
ESP32-EVB canonical map :1502  --read FC03-->  ROCK Pi node poller
ROCK Pi northbound map :1502   --read FC04-->  защитен backend/OpenRemote agent
Planned forwarding: ROCK Pi -> Site Router VPN -> private MQTT -> backend ingestion

Suntech STE-261L :3200         --read FC01/FC03/FC04--> ROCK Pi driver
```

ROCK Pi е единственият разрешен Modbus TCP client към ESP32 нод. Нодът приема
TCP връзка само след local serial provisioning:

```text
rockpi <ROCK_PI_IPV4>
status
```

Това записва в ESP32 NVS единствено разрешения source адрес. Не записва парола,
не включва MQTT, не избира driver и не разрешава команда. `status` показва UID,
identity, Ethernet адреса и настроения ROCK Pi source на local USB serial
конзолата.

Директният ESP32 MQTT е изключен дори при стара NVS настройка за включване.
MQTT публикуването от ROCK Pi е отделна оставаща задача; пилотът провери само
Modbus четене.

### Препращане на състояние

ROCK Pi чете ESP32 holding регистрите `0x0000..0x000C` и `0x0040..0x0045`.
Той препраща online state, identity, driver ID, quality, heartbeat, power,
energy, device state, alarms, data age и MQTT connection state в northbound
input-register node slots от `0x0100`.

ESP32 command window остава `0x0014..0x0017`, но bench процесът не пише в него.
Нод с текущия unconfigured driver отказва всяка power команда, дори клиент да
опита да изпрати такава.

### Suntech настройки и състояние

Suntech не се маршрутизира през ESP32. ROCK Pi го чете директно по Modbus TCP.
Northbound картата му препраща само за четене PCS on state, grid-tied state,
current-source mode, SOC upper/lower limits, BMS charge/discharge limits,
status/fault flags и telemetry. Accumulated charge/discharge energy регистрите
`122..125` се четат като една FC04 заявка, за да са общ snapshot.

| Northbound input адрес | Read-only Suntech стойност |
|---:|---|
| `5`, `6` | live BMS максимална charge / discharge kW ×10 |
| `7`, `8`, `25` | BMS status, system flags и alarm bits |
| `29`, `30` | наблюдавани SOC upper / lower limits (%) |
| `32`, `33`, `34` | PCS power-on, grid-tied и current-source state |

В този bench договор няма Suntech write, heartbeat enable, ESP32 команда, MQTT
secret или backend адрес.
