# Firmware architecture

~~~text
OpenRemote Modbus TCP Agent
          |
          v
Northbound register server (stable canonical map)
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
Device driver interface
  - SunStorage Pro 261 / STE-261L
  - future Sinexcel, Deye, Sungrow, Huawei, Growatt
          |
          v
Modbus TCP/RTU transport + HAL
~~~

Драйверите преобразуват vendor адреси, signedness, scale, byte order и sign convention. Останалият core работи само с канонични единици и не знае марката на устройството.
