#include "crc.h"

uint32_t crc24(const uint8_t *data, size_t length)
{
    uint32_t crc = 0x000000U;
    for (size_t i = 0; i < length; i++) {
        crc ^= ((uint32_t)data[i]) << 16;
        for (int bit = 0; bit < 8; bit++) {
            crc <<= 1;
            if (crc & 0x1000000U)
                crc ^= CRC24_POLY;
        }
    }
    return crc & 0xFFFFFFU;
}
