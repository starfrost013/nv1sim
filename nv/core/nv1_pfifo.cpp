//
// nv1_pfifo.cpp
// NV1 Object Submission System
//

#include <nv/nv1.hpp>

namespace NV1Sim
{
    #define NV1_GRAY_TABLE_NUM_ENTRIES      32

    uint8_t nv1_pfifo_cache1_gray_code_table[NV1_GRAY_TABLE_NUM_ENTRIES] = {
        0b000000, 0b000001, 0b000011, 0b000010, 0b000110, 0b000111, 0b000101, 0b000100, //0x07
        0b001100, 0b001101, 0b001111, 0b001110, 0b001010, 0b001011, 0b001001, 0b001000, //0x0F
        0b011000, 0b011001, 0b011011, 0b011010, 0b011110, 0b011111, 0b011101, 0b011100, //0x17
        0b010100, 0b010101, 0b010111, 0b010110, 0b010010, 0b010011, 0b010001, 0b010000, //0x1F
    };

    /* The function is called up to hundreds of thousands of times per second, it's too slow to do anything else */
    uint8_t nv1_pfifo_cache1_binary_code_table[NV1_GRAY_TABLE_NUM_ENTRIES] =
    {
        0x00, 0x01, 0x03, 0x02, 0x07, 0x06, 0x04, 0x05, // 0x07 (0)
        0x0F, 0x0E, 0x0C, 0x0D, 0x08, 0x09, 0x0B, 0x0A, // 0x0F (1000)
        0x1F, 0x1E, 0x1C, 0x1D, 0x18, 0x19, 0x1B, 0x1A, // 0x17 (10000)
        0x10, 0x11, 0x13, 0x12, 0x17, 0x16, 0x14, 0x15, // 0x1F (11000)
    };
}