#include "crc_ilp.h"
#include "crc.h"

/* Same 256-entry LUT as V1 — shared concept, independent accumulators */
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

/**
 * CRC combination: given CRC of block A and CRC of block B (length n),
 * compute CRC of A||B.
 *
 * This is needed to merge 4 independent accumulator streams at the end.
 * We shift crc_a left by n bytes (in CRC space) then XOR with crc_b.
 */
static uint32_t crc24_shift(uint32_t crc, size_t n)
{
    /* Shift CRC left by n bytes using the LUT */
    for (size_t i = 0; i < n; i++) {
        uint8_t pos = (uint8_t)(crc >> 16);
        crc = ((crc << 8) ^ lut[pos]) & 0xFFFFFFU;
    }
    return crc;
}

uint32_t crc24_ilp(const uint8_t *data, size_t length)
{
    if (!lut_ready) build_lut();

    /* Fall back to single accumulator for short inputs */
    if (length < 4) {
        uint32_t crc = 0;
        for (size_t i = 0; i < length; i++) {
            uint8_t pos = (uint8_t)((crc >> 16) ^ data[i]);
            crc = ((crc << 8) ^ lut[pos]) & 0xFFFFFFU;
        }
        return crc;
    }

    /*
     * Split input into 4 equal chunks, process independently.
     *
     * data: [chunk0 | chunk1 | chunk2 | chunk3 | tail]
     *
     * Each accumulator processes its chunk with no dependency on others.
     * The CPU's out-of-order execution engine can issue all 4 LUT lookups
     * in parallel since crc0..crc3 are independent registers.
     */
    size_t chunk = length / 4;
    size_t tail_start = chunk * 4;

    const uint8_t *p0 = data;
    const uint8_t *p1 = data + chunk;
    const uint8_t *p2 = data + chunk * 2;
    const uint8_t *p3 = data + chunk * 3;

    uint32_t crc0 = 0, crc1 = 0, crc2 = 0, crc3 = 0;

    /* Process 4 bytes per loop iteration — one per accumulator */
    for (size_t i = 0; i < chunk; i++) {
        uint8_t pos0 = (uint8_t)((crc0 >> 16) ^ p0[i]);
        uint8_t pos1 = (uint8_t)((crc1 >> 16) ^ p1[i]);
        uint8_t pos2 = (uint8_t)((crc2 >> 16) ^ p2[i]);
        uint8_t pos3 = (uint8_t)((crc3 >> 16) ^ p3[i]);

        crc0 = ((crc0 << 8) ^ lut[pos0]) & 0xFFFFFFU;
        crc1 = ((crc1 << 8) ^ lut[pos1]) & 0xFFFFFFU;
        crc2 = ((crc2 << 8) ^ lut[pos2]) & 0xFFFFFFU;
        crc3 = ((crc3 << 8) ^ lut[pos3]) & 0xFFFFFFU;
    }

    /*
     * Combine 4 independent CRCs into one.
     *
     * CRC is linear: CRC(A||B) = CRC(A) shifted by len(B) XOR CRC(B)
     *
     * crc0 covers bytes [0 .. chunk-1]        → shift by 3*chunk
     * crc1 covers bytes [chunk .. 2chunk-1]   → shift by 2*chunk
     * crc2 covers bytes [2chunk .. 3chunk-1]  → shift by 1*chunk
     * crc3 covers bytes [3chunk .. 4chunk-1]  → shift by 0
     */
    uint32_t crc = crc24_shift(crc0, 3 * chunk)
                 ^ crc24_shift(crc1, 2 * chunk)
                 ^ crc24_shift(crc2, 1 * chunk)
                 ^ crc3;

    /* Handle remaining tail bytes (length % 4) */
    for (size_t i = tail_start; i < length; i++) {
        uint8_t pos = (uint8_t)((crc >> 16) ^ data[i]);
        crc = ((crc << 8) ^ lut[pos]) & 0xFFFFFFU;
    }

    return crc;
}
