#include "crc_table.h"
#include "crc.h"

/* Precomputed lookup table: crc24 of each single byte 0x00..0xFF */
static uint32_t lut[256];
static int lut_ready = 0;

static void build_lut(void)
{
    for (int i = 0; i < 256; i++) {
        uint8_t b = (uint8_t)i;
        lut[i] = crc24(&b, 1);
    }
    lut_ready = 1;
}

uint32_t crc24_table(const uint8_t *data, size_t length)
{
    if (!lut_ready) build_lut();

    uint32_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        /* XOR top byte of CRC with input byte, look up remainder,
         * then shift CRC left by 8 and XOR with table entry */
        uint8_t pos = (uint8_t)((crc >> 16) ^ data[i]);
        crc = ((crc << 8) ^ lut[pos]) & 0xFFFFFFU;
    }
    return crc;
}
