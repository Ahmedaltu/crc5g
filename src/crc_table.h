#ifndef CRC_TABLE_H
#define CRC_TABLE_H

#include <stdint.h>
#include <stddef.h>

/**
 * CRC-24 using a 256-entry lookup table (table-driven method).
 *
 * Instead of processing one bit at a time (8 iterations per byte),
 * we precompute CRC values for all 256 possible byte values.
 * Each byte lookup replaces 8 bit-loop iterations → ~8x fewer operations.
 *
 * This is the same optimisation used in production 5G baseband software.
 */
uint32_t crc24_table(const uint8_t *data, size_t length);

#endif
