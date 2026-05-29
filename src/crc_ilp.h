#ifndef CRC_ILP_H
#define CRC_ILP_H

#include <stdint.h>
#include <stddef.h>

/**
 * CRC-24 V2: Instruction-Level Parallelism (ILP) with 4 independent accumulators.
 *
 * Problem with V1 (LUT): one accumulator creates a dependency chain —
 * each iteration must wait for the previous CRC value before it can proceed.
 *
 *   V1 dependency chain (serial):
 *   crc = f(crc, data[0])   ← must finish before...
 *   crc = f(crc, data[1])   ← ...this can start
 *   crc = f(crc, data[2])   ← ...and this
 *
 * V2 solution: 4 independent accumulators processing 4 bytes simultaneously.
 * The CPU can execute all 4 in parallel since they don't depend on each other.
 *
 *   V2 independent (parallel):
 *   crc0 = f(crc0, data[0])  ─┐
 *   crc1 = f(crc1, data[1])  ─┤ all 4 execute in parallel
 *   crc2 = f(crc2, data[2])  ─┤
 *   crc3 = f(crc3, data[3])  ─┘
 *
 * This is the same technique as V2 in CS-E4580 (vv[nb] accumulators).
 * Expected speedup over V1: 2-4x depending on CPU pipeline depth.
 */
uint32_t crc24_ilp(const uint8_t *data, size_t length);

#endif
