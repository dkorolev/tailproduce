// Assert that the architecture is little endian to ensure serialization compatibility.

#include <gtest/gtest.h>

TEST(ArchIsLittleEndian, Int16) {
    uint16_t v = 0x1234;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0x34, p[0]);
    EXPECT_EQ(0x12, p[1]);
}

TEST(ArchIsLittleEndian, Int32) {
    uint32_t v = 0x87654321;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0x21, p[0]);
    EXPECT_EQ(0x43, p[1]);
    EXPECT_EQ(0x65, p[2]);
    EXPECT_EQ(0x87, p[3]);
}

TEST(ArchIsLittleEndian, Int64) {
    uint64_t v = 0xdeadbeef12345678;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0x78, p[0]);
    EXPECT_EQ(0x56, p[1]);
    EXPECT_EQ(0x34, p[2]);
    EXPECT_EQ(0x12, p[3]);
    EXPECT_EQ(0xef, p[4]);
    EXPECT_EQ(0xbe, p[5]);
    EXPECT_EQ(0xad, p[6]);
    EXPECT_EQ(0xde, p[7]);
}
