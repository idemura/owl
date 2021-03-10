#include "foundation/string.h"

#include <gtest/gtest.h>

TEST(string, basic_ops)
{
    string s = string_copy_of("hello string");
    EXPECT_EQ(12, s.len);
    EXPECT_STREQ("hello string", s.str);
    string_release(s);
}
