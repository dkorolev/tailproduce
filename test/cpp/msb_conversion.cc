// Ensure that MSB (big endian) order is correct and follows the lexicographical one.
// Lexicographical order is essential for time keys encoding in the storage proxy.
// Also implicitly tests that the host order is little endian.

#include <gtest/gtest.h>

#include <vector>

#include "../../src/helpers.h"

#include "../../src/byte_order.h"

using ::TailProduce::bytes;

TEST(MSBConversion, UInt16) {
    uint16_t v = as_msb(static_cast<uint16_t>(0x1234));
    EXPECT_NE(0x1234, v);
    EXPECT_EQ(0x1234, msb_to_host_order(v));
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0x12, p[0]);
    EXPECT_EQ(0x34, p[1]);
}

TEST(MSBConversion, Int16) {
    int16_t v = as_msb(static_cast<int16_t>(0x1234));
    EXPECT_NE(0x1234, v);
    EXPECT_EQ(0x1234, msb_to_host_order(v));
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0x12, p[0]);
    EXPECT_EQ(0x34, p[1]);
}

TEST(MSBConversion, UInt32) {
    uint32_t v = as_msb(static_cast<uint32_t>(0x87654321));
    EXPECT_NE(0x87654321, v);
    EXPECT_EQ(0x87654321, msb_to_host_order(v));
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0x87, p[0]);
    EXPECT_EQ(0x65, p[1]);
    EXPECT_EQ(0x43, p[2]);
    EXPECT_EQ(0x21, p[3]);
}

TEST(MSBConversion, Int32) {
    int32_t v = as_msb(static_cast<int32_t>(0x87654321));
    EXPECT_NE(0x87654321, v);
    EXPECT_EQ(0x87654321, msb_to_host_order(v));
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0x87, p[0]);
    EXPECT_EQ(0x65, p[1]);
    EXPECT_EQ(0x43, p[2]);
    EXPECT_EQ(0x21, p[3]);
}

TEST(MSBConversion, UInt64) {
    uint64_t v = as_msb(static_cast<uint64_t>(0xdeadbeef12345678));
    EXPECT_NE(0xdeadbeef12345678, v);
    EXPECT_EQ(0xdeadbeef12345678, msb_to_host_order(v));
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0xde, p[0]);
    EXPECT_EQ(0xad, p[1]);
    EXPECT_EQ(0xbe, p[2]);
    EXPECT_EQ(0xef, p[3]);
    EXPECT_EQ(0x12, p[4]);
    EXPECT_EQ(0x34, p[5]);
    EXPECT_EQ(0x56, p[6]);
    EXPECT_EQ(0x78, p[7]);
}

TEST(MSBConversion, Int64) {
    int64_t v = as_msb(static_cast<int64_t>(0xdeadbeef12345678));
    EXPECT_NE(0xdeadbeef12345678, v);
    EXPECT_EQ(0xdeadbeef12345678, msb_to_host_order(v));
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&v);
    EXPECT_EQ(0xde, p[0]);
    EXPECT_EQ(0xad, p[1]);
    EXPECT_EQ(0xbe, p[2]);
    EXPECT_EQ(0xef, p[3]);
    EXPECT_EQ(0x12, p[4]);
    EXPECT_EQ(0x34, p[5]);
    EXPECT_EQ(0x56, p[6]);
    EXPECT_EQ(0x78, p[7]);
}

TEST(MSBConversion, OrdersUInt16) {
    const uint16_t a = 0x1020, b = 0x2010;
    ASSERT_LT(bytes(as_msb(a)), bytes(as_msb(b)));
    ASSERT_GT(bytes(a), bytes(b));
}

TEST(MSBConversion, OrdersInt16) {
    const int16_t a = 0x1020, b = 0x2010;
    ASSERT_LT(bytes(as_msb(a)), bytes(as_msb(b)));
    ASSERT_GT(bytes(a), bytes(b));
}

TEST(MSBConversion, OrdersUInt32) {
    const uint32_t a = 0x12345678, b = 0x87654321;
    ASSERT_LT(bytes(as_msb(a)), bytes(as_msb(b)));
    ASSERT_GT(bytes(a), bytes(b));
}

TEST(MSBConversion, OrdersInt32) {
    const int32_t a = 0x12345678, b = 0x87654321;
    ASSERT_LT(bytes(as_msb(a)), bytes(as_msb(b)));
    ASSERT_GT(bytes(a), bytes(b));
}

TEST(MSBConversion, OrdersUInt64) {
    const uint64_t a = 0x1122334455667788, b = 0x8877665544332211;
    ASSERT_LT(bytes(as_msb(a)), bytes(as_msb(b)));
    ASSERT_GT(bytes(a), bytes(b));
}

TEST(MSBConversion, OrdersInt64) {
    const int64_t a = 0x1122334455667788, b = 0x8877665544332211;
    ASSERT_LT(bytes(as_msb(a)), bytes(as_msb(b)));
    ASSERT_GT(bytes(a), bytes(b));
}
