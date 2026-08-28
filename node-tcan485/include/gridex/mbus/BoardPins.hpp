#pragma once

namespace gridex::mbus::board {

// Official LilyGo T-CAN485 pin mapping.
inline constexpr int Rs485Tx = 22;
inline constexpr int Rs485Rx = 21;
inline constexpr int Rs485Callback = 17;
inline constexpr int Rs485Enable = 9;
inline constexpr int BoosterEnable = 16;
inline constexpr int StatusLed = 4;

inline constexpr unsigned long Baud = 115200;
inline constexpr unsigned long FrameSilenceUs = 4000;

}  // namespace gridex::mbus::board
