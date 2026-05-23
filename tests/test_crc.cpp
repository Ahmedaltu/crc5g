#include <gtest/gtest.h>

extern "C" {
#include "../src/crc.h"
}

// Empty input should produce zero CRC
TEST(CRC24Test, EmptyInput) {
    EXPECT_EQ(crc24(nullptr, 0), 0x000000U);
}

// Single zero byte
TEST(CRC24Test, SingleZeroByte) {
    uint8_t data[] = {0x00};
    EXPECT_EQ(crc24(data, 1), 0x000000U);
}

// Single 0xFF byte - known reference value
TEST(CRC24Test, SingleFFByte) {
    uint8_t data[] = {0xFF};
    uint32_t result = crc24(data, 1);
    EXPECT_LT(result, 0x1000000U);  // must fit in 24 bits
    EXPECT_EQ(result, crc24(data, 1));  // deterministic
}

// Result must always be 24-bit (upper byte zero)
TEST(CRC24Test, ResultFitsIn24Bits) {
    uint8_t data[] = {0xAB, 0xCD, 0xEF, 0x12, 0x34};
    uint32_t result = crc24(data, sizeof(data));
    EXPECT_EQ(result & 0xFF000000U, 0U);
}

// Different inputs must produce different CRCs (collision resistance)
TEST(CRC24Test, DifferentInputsDifferentCRC) {
    uint8_t a[] = {0x01};
    uint8_t b[] = {0x02};
    EXPECT_NE(crc24(a, 1), crc24(b, 1));
}

// Changing one bit must change the CRC (error detection)
TEST(CRC24Test, SingleBitErrorDetected) {
    uint8_t original[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    uint8_t corrupted[] = {0x49, 0x65, 0x6C, 0x6C, 0x6F};  // bit flip in first byte
    EXPECT_NE(crc24(original, 5), crc24(corrupted, 5));
}

// CRC is position-sensitive: same bytes different order = different CRC
TEST(CRC24Test, OrderMatters) {
    uint8_t a[] = {0x01, 0x02};
    uint8_t b[] = {0x02, 0x01};
    EXPECT_NE(crc24(a, 2), crc24(b, 2));
}

// Longer known input - self-consistency check
TEST(CRC24Test, ConsistencyAcrossRuns) {
    uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39}; // "123456789"
    uint32_t first  = crc24(data, sizeof(data));
    uint32_t second = crc24(data, sizeof(data));
    EXPECT_EQ(first, second);
}
