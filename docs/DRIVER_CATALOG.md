# GrideX node driver catalog contract

## Rule

One `driver_id` means one exact hardware/protocol profile:

~~~text
driver_id
  -> node type
  -> manufacturer
  -> exact model or model family with identical map
  -> protocol document and revision/date
  -> RS485 baud/data/parity/stop
  -> Modbus unit ID policy
  -> register map, offset, scale, byte order and sign convention
  -> allowed write operations and commissioning evidence
~~~

Names such as "Deye", "Sungrow", "Huawei" or "Growatt" are not sufficient driver identities. Different models and firmware revisions can use different maps.

## ID ranges

| Range | Node class |
|---|---|
| 1000-1999 | Inverters |
| 2000-2999 | Battery/BMS |
| 3000-3999 | All-in-one BESS / PCS |
| 4000-4999 | Smart meters |
| 5000-5999 | EV charging stations |

IDs are allocated only after the source register map is archived in the project and the read-only mapping has passed a bench test. Writes remain locked until address, unit ID, sign, scale and safety limits are confirmed on the actual device.

## Required catalog entry

~~~yaml
driver_id: 1001
node_type: inverter
manufacturer: REQUIRED
model: REQUIRED
protocol:
  document: REQUIRED
  revision: REQUIRED
serial:
  baud: REQUIRED
  data_bits: 8
  parity: REQUIRED
  stop_bits: REQUIRED
  unit_id: REQUIRED
commissioning:
  read_mapping_confirmed: false
  write_mapping_confirmed: false
  sign_confirmed: false
  scale_confirmed: false
~~~
