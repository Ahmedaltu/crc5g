#include <gtest/gtest.h>

extern "C" {
#include "../src/crc.h"
#include "../src/crc_table.h"
}

// Table version: empty input should produce zero CRC
TEST(CRC24TableTest, EmptyInput) {
    EXPECT_EQ(crc24_table(nullptr, 0), 0x000000U);
}

// Single zero byte
TEST(CRC24TableTest, SingleZeroByte) {
    uint8_t data[] = {0x00};
    EXPECT_EQ(crc24_table(data, 1), 0x000000U);
}

// Result must always fit in 24 bits
TEST(CRC24TableTest, ResultFitsIn24Bits) {
    uint8_t data[] = {0xAB, 0xCD, 0xEF, 0x12, 0x34};
    uint32_t result = crc24_table(data, sizeof(data));
    EXPECT_EQ(result & 0xFF000000U, 0U);
}

// Deterministic across runs
TEST(CRC24TableTest, ConsistencyAcrossRuns) {
    uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35};
    EXPECT_EQ(crc24_table(data, sizeof(data)), crc24_table(data, sizeof(data)));
}

// Single bit error must be detected
TEST(CRC24TableTest, SingleBitErrorDetected) {
    uint8_t original[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    uint8_t corrupted[] = {0x49, 0x65, 0x6C, 0x6C, 0x6F};  // bit flip
    EXPECT_NE(crc24_table(original, 5), crc24_table(corrupted, 5));
}

// KEY TEST: naive and table must produce identical results for all inputs
TEST(CRC24TableTest, MatchesNaiveImplementation_SingleByte) {
    for (int i = 0; i < 256; i++) {
        uint8_t b = (uint8_t)i;
        EXPECT_EQ(crc24(&b, 1), crc24_table(&b, 1))
            << "Mismatch at byte 0x" << std::hex << i;
    }
}

TEST(CRC24TableTest, MatchesNaiveImplementation_MultiiByte) {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0xFF, 0x5A, 0xA5};
    EXPECT_EQ(crc24(data, sizeof(data)), crc24_table(data, sizeof(data)));
}

TEST(CRC24TableTest, MatchesNaiveImplementation_LongInput) {
    uint8_t data[1024];
    for (int i = 0; i < 1024; i++)
        data[i] = (uint8_t)(i & 0xFF);
    EXPECT_EQ(crc24(data, sizeof(data)), crc24_table(data, sizeof(data)));
}
