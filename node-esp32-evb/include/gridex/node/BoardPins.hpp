#pragma once

namespace gridex::node::board {

// Official OLIMEX ESP32-EVB CAN mapping from the vendor's ESP-IDF TWAI demo.
inline constexpr int CanTx = 5;
inline constexpr int CanRx = 35;

// Direct UART pins for a simple external RS485 transceiver only. Do not use
// this path for OLIMEX MOD-RS485-ISO: that module has a PIC firmware bridge
// and is controlled through the UEXT RS232/I2C host protocol; see
// docs/OLIMEX_MOD_RS485_ISO.md before adding its dedicated adapter.
inline constexpr int Rs485Tx = 4;
inline constexpr int Rs485Rx = 36;
// UEXT GPIO13 is used as the default direction-control output by the
// direct-transceiver carrier. It remains board-profile configurable at build
// time because another carrier may use a different GPIO.
inline constexpr int Rs485Direction = 13;

inline constexpr int Relay1 = 32;
inline constexpr int Relay2 = 33;
inline constexpr int UserButton = 34;

}  // namespace gridex::node::board
