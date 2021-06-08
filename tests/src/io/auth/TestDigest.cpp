#include <dots/testing/gtest/gtest.h>
#include <dots/io/auth/Digest.h>

TEST(TestDigest, stringHashValue1)
{
    dots::io::Digest digest{ dots::io::Nonce{ 0 }, "cnonce", "clientName", "secret" };
    EXPECT_EQ(digest.toString(), "f374890c4765d9da457724f82f516f4ebc874a24b4ea5a2a0f983922aa0b8149");
}

TEST(TestDigest, stringHashValue2)
{
    dots::io::Digest digest{ dots::io::Nonce{ 5 }, "cnonce", "clientName", "secret" };
    EXPECT_EQ(digest.toString(), "de10dac51d89b3e206e47d29d270472dca04cc860d71ec3cb563bc619ff7794d");
}

TEST(TestDigest, nonceRoundtrip)
{
    EXPECT_EQ(dots::io::Nonce{ 0x123456789 }.toString(), "0000000123456789");
    EXPECT_EQ(dots::io::Nonce{ "0000000123456789" }.value(), 0x123456789u);
}

TEST(TestDigest, nonceSize)
{
    auto cnonce = dots::io::Nonce{}.toString();
    EXPECT_EQ(cnonce.size(), 16);
}