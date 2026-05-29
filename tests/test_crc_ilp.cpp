#include <gtest/gtest.h>

extern "C" {
#include "../src/crc.h"
#include "../src/crc_ilp.h"
}

// Result must fit in 24 bits
TEST(CRC24ILPTest, ResultFitsIn24Bits) {
    uint8_t data[] = {0xAB, 0xCD, 0xEF, 0x12, 0x34};
    EXPECT_EQ(crc24_ilp(data, sizeof(data)) & 0xFF000000U, 0U);
}

// Deterministic
TEST(CRC24ILPTest, Deterministic) {
    uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35};
    EXPECT_EQ(crc24_ilp(data, sizeof(data)), crc24_ilp(data, sizeof(data)));
}

// Single bit error detected
TEST(CRC24ILPTest, SingleBitErrorDetected) {
    uint8_t original[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    uint8_t corrupted[] = {0x49, 0x65, 0x6C, 0x6C, 0x6F};
    EXPECT_NE(crc24_ilp(original, 5), crc24_ilp(corrupted, 5));
}

// Short input (< 4 bytes) falls back correctly
TEST(CRC24ILPTest, ShortInputMatchesNaive) {
    uint8_t data[] = {0xAB, 0xCD};
    EXPECT_EQ(crc24(data, sizeof(data)), crc24_ilp(data, sizeof(data)));
}

// KEY: must match naive for all 256 single bytes
TEST(CRC24ILPTest, MatchesNaive_AllSingleBytes) {
    for (int i = 0; i < 256; i++) {
        uint8_t b = (uint8_t)i;
        EXPECT_EQ(crc24(&b, 1), crc24_ilp(&b, 1))
            << "Mismatch at byte 0x" << std::hex << i;
    }
}

// Must match naive for multi-byte input
TEST(CRC24ILPTest, MatchesNaive_MultiByte) {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0xFF, 0x5A, 0xA5};
    EXPECT_EQ(crc24(data, sizeof(data)), crc24_ilp(data, sizeof(data)));
}

// Must match naive for 1KB input (exercises chunk splitting)
TEST(CRC24ILPTest, MatchesNaive_1KB) {
    uint8_t data[1024];
    for (int i = 0; i < 1024; i++) data[i] = (uint8_t)(i & 0xFF);
    EXPECT_EQ(crc24(data, sizeof(data)), crc24_ilp(data, sizeof(data)));
}

// Must match naive for non-multiple-of-4 length (exercises tail handling)
TEST(CRC24ILPTest, MatchesNaive_NonMultipleOf4) {
    uint8_t data[101];
    for (int i = 0; i < 101; i++) data[i] = (uint8_t)(i * 3);
    EXPECT_EQ(crc24(data, sizeof(data)), crc24_ilp(data, sizeof(data)));
}
