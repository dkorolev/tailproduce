#include <gtest/gtest.h>

#include "../../src/fixed_size_serializer.h"

using ::TailProduce::FixedSizeSerializer;

TEST(ArchIsLittleEndian, UInt16) {
    EXPECT_EQ(5, FixedSizeSerializer<uint16_t>::size_in_bytes);
    // Does not fit signed 16-bit, requires unsigned.
    EXPECT_EQ("54321", FixedSizeSerializer<uint16_t>::PackToString(54321));
    EXPECT_EQ(54321, FixedSizeSerializer<uint16_t>::UnpackFromString("54321"));
}

TEST(ArchIsLittleEndian, UInt32) {
    EXPECT_EQ(10, FixedSizeSerializer<uint32_t>::size_in_bytes);
    // Does not fit signed 32-bit, requires unsigned.
    EXPECT_EQ("3987654321", FixedSizeSerializer<uint32_t>::PackToString(3987654321));
    EXPECT_EQ(3987654321, FixedSizeSerializer<uint32_t>::UnpackFromString("3987654321"));
}

TEST(ArchIsLittleEndian, UInt64) {
    EXPECT_EQ(20, FixedSizeSerializer<uint64_t>::size_in_bytes);
    uint64_t magic = 1e19;
    magic += 42;
    // Does not fit unsigned 64-bit.
    EXPECT_EQ("10000000000000000042", FixedSizeSerializer<uint64_t>::PackToString(magic));
    EXPECT_EQ(magic, FixedSizeSerializer<uint64_t>::UnpackFromString("10000000000000000042"));
}
