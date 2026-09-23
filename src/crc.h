#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stddef.h>

/* CRC-24B as used for code-block CRC in 5G NR (3GPP TS 38.212). */
#define CRC24_POLY 0x800063U

uint32_t crc24(const uint8_t *data, size_t length);

#endif
