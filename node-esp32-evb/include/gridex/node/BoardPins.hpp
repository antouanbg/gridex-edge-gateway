#pragma once

namespace gridex::node::board {

// Official OLIMEX ESP32-EVB CAN mapping from the vendor's ESP-IDF TWAI demo.
inline constexpr int CanTx = 5;
inline constexpr int CanRx = 35;

// UEXT UART pins used by the optional external, galvanically isolated RS485
// transceiver. The exact carrier revision must validate these assignments.
inline constexpr int Rs485Tx = 4;
inline constexpr int Rs485Rx = 36;
// UEXT GPIO13 is used as the default direction-control output by the
// production RS485 carrier. It remains board-profile configurable at build
// time because a third-party isolated carrier may use another GPIO.
inline constexpr int Rs485Direction = 13;

inline constexpr int Relay1 = 32;
inline constexpr int Relay2 = 33;
inline constexpr int UserButton = 34;

}  // namespace gridex::node::board
