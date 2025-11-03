// 
// The NV1 emulator (The real one!)
// Various utility methods
//

#pragma once

#include <cstdint>

namespace NV1Sim
{
    uint8_t Util_Gray2Binary(uint32_t gray);                // Convert a gray code number into binary
    uint8_t Util_Binary2Gray(uint32_t gray);                // Convert binary code number into gray
}