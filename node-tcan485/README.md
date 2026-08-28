# T-CAN485 MBUS node firmware

One ESP32 firmware image for all GrideX expansion nodes. The configured `node_type` selects the active device driver; vendor-specific mappings never enter the ROCK Pi E base code.

## Board pins

The firmware uses the official LilyGo T-CAN485 mapping:

- RS485 TX GPIO22
- RS485 RX GPIO21
- CALLBACK GPIO17
- RS485 enable GPIO9
- booster enable GPIO16
- WS2812/status GPIO4

The onboard MAX13487 provides the RS485 physical layer. Bus settings are 115200 baud, 8N1.

## Node types

- `4` - industrial meter
- `5` - EV charging station
- `6` - second BESS cabinet

The initial implementation intentionally uses `UnconfiguredDriver` for all three types. It exposes identity/configuration and MBUS transport safely, but does not write to downstream equipment until the corresponding manufacturer register map is approved.

## Addressing

An uncommissioned node derives a temporary address in the reserved range 200-247 from its ESP32 eFuse UID. The base scans that range, reads registers 0x0006-0x0009 (UID), then writes final address, type and apply key `0xA55A`. The result is stored in ESP32 Preferences; no DIP switches are needed.

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
