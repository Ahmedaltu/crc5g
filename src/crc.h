#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stddef.h>

/* CRC-24 as used in 5G NR polar codes (3GPP TS 38.212)
 * Generator polynomial: g(x) = x^24 + x^23 + x^6 + x^5 + x + 1 */
#define CRC24_POLY 0x800063U

uint32_t crc24(const uint8_t *data, size_t length);

#endif
