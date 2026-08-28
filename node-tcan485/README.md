# T-CAN485 MBUS node firmware

ESP32 firmware framework for GrideX expansion nodes. Every commissioned node receives both `node_type` and a mandatory `driver_id`. The `driver_id` identifies one exact manufacturer, model and protocol-map revision; vendor-specific mappings never enter the ROCK Pi E base code.

Production firmware may contain only the approved driver for that node, or a signed catalog of drivers. In both cases exactly one concrete driver is active.

## Board pins

The firmware uses the official LilyGo T-CAN485 mapping:

- RS485 TX GPIO22
- RS485 RX GPIO21
- CALLBACK GPIO17
- RS485 enable GPIO9
- booster enable GPIO16
- WS2812/status GPIO4

The onboard MAX13487 provides the RS485 physical layer. Bus settings are 115200 baud, 8N1.

## Direct OpenRemote telemetry

Every node can additionally publish its latest sample directly to a private
OpenRemote installation. The transport is MQTT over TLS on port `8883`; plain
MQTT port `1883` is intentionally unsupported by this firmware. OpenRemote
Manager is the broker, so no separate Mosquitto service is required.

The node publishes every two seconds to:

```text
{realm}/{clientId}/writeattributevalue/{attributeName}/{assetId}
```

Attributes are `online`, `actualPowerKw`, `energyWh`, `deviceState`,
`alarmBits`, `quality` and `heartbeat`. An MQTT last will sets `online=false`
after an unexpected disconnect. The TLS server CA is mandatory and the
firmware never falls back to an insecure connection.

Commissioning writes these ESP32 Preferences values in namespace
`gridex-cloud`: `enabled`, `wifi_ssid`, `wifi_pass`, `mqtt_host`, `mqtt_port`,
`realm`, `mqtt_user`, `mqtt_secret`, `client_id`, `asset_id`, `ca_cert`.
Use a unique client ID and restricted OpenRemote service user per node. Secrets
must be injected during commissioning and must not be committed to Git.

This cloud path is telemetry-only. All control continues through OpenRemote ->
ROCK Pi E -> local safety envelope. If Wi-Fi or the cloud fails, MBUS polling
and the downstream device driver continue locally.

## Two isolated serial sides for RS485 equipment

An inverter node cannot use one transceiver for both the MBUS backbone and the inverter. It requires:

- RS485-A: onboard MAX13487, upstream MBUS server toward ROCK Pi E;
- RS485-B: additional isolated transceiver on a node carrier/daughterboard, downstream Modbus client toward the concrete inverter.

The two buses may have different baud, parity, unit ID and termination. CAN is not used as a substitute for the second RS485 channel.

## Node types

- `1` - inverter
- `2` - battery/BMS
- `3` - all-in-one BESS
- `4` - industrial meter
- `5` - EV charging station
- `6` - second BESS cabinet

The initial implementation intentionally uses `UnconfiguredDriver`. It exposes identity/configuration and MBUS transport safely, but does not write to downstream equipment until the exact manufacturer/model register map is approved and assigned a `driver_id`.

## Addressing

An uncommissioned node derives a temporary address in the reserved range 200-247 from its ESP32 eFuse UID. The base scans that range, reads the UID, then writes final address, node type, concrete driver ID and apply key `0xA55A`. The result is stored in ESP32 Preferences; no DIP switches are needed.

Temporary-address collisions remain possible. Production discovery must therefore retry with one node connected at a time or add the slotted UID discovery extension defined in the final MBUS document.

## Build and flash

~~~bash
cd node-tcan485
pio run
pio run -t upload
pio device monitor
~~~

Host protocol tests do not need PlatformIO:

~~~bash
cmake -S node-tcan485 -B build-node
cmake --build build-node
ctest --test-dir build-node --output-on-failure
~~~
