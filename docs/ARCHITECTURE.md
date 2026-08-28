# Firmware architecture

~~~text
OpenRemote / strategy service (WAN)
          |
          v
ROCK Pi E northbound register server (stable canonical map)
          |
          v
Command arbiter + EMS timeout
          |
          v
Safety envelope
  - BMS charge/discharge limits
  - charge/discharge prohibit bits
  - software fuse / PCC headroom
  - ramp and SOC rules (next increment)
          |
          v
Transport and driver boundary
  - GbE/OT: SunStorage Pro 261 / STE-261L driver on ROCK Pi E
  - RS485/MBUS: canonical register map from T-CAN485 nodes
          |
          v
Dual Ethernet + isolated RS485 HAL
~~~

STE-261L е директен TCP driver в базовия модул, защото кабинетът е в OT Ethernet мрежата. Марковата специфика на RS485 периферията живее само в T-CAN485 нодовете. ROCK Pi E вижда всички нодове през една канонична MBUS карта и не знае марката на електромера/зарядното.

## Physical data paths

~~~text
                         WAN / Internet
OpenRemote + IBEX <---- 100 MbE
                           |
                    +------v-------+
                    |  ROCK Pi E   |
                    |  RK3328      |
                    +--+--------+--+
        GbE / OT -------+        +------ isolated RS485-A / 12 V MBUS
             |                             |       |       |
        STE-261L:3200                 inverter   EVSE   meter node
                                         |
                                  isolated RS485-B
                                         |
                                  concrete inverter
~~~

Route policy: default route exists only on WAN. The OT interface has a static address and a connected route only. Firewall rules reject forwarding between WAN and OT; the local gateway service is the only allowed application path.

Each node exposes `node_type` plus `driver_id`. `driver_id` is not a generic vendor selector: it binds manufacturer, exact model, protocol revision, serial profile, register map, sign convention and scaling. A driver mismatch keeps the downstream write path locked.
