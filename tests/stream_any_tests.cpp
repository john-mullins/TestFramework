#include "MiniTestFramework.h"
#include <sstream>
#include <string>
#include <string_view>

#ifdef __has_include             // Check if __has_include is present
#if __has_include(<string_view>) // Check for a standard library
#include <string_view>
#define HAS_STRING_VIEW
#endif
#endif

using namespace std::literals;

TEST(string)
{
    auto              s{"Hello, World"s};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("Hello, World", ss.str());
}

TEST(const_char_array)
{
    const char        s[]{'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd'};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("Hello, World", ss.str());
}

#ifdef HAS_STRING_VIEW
TEST(string_view)
{
    auto              s{"Hello, World"sv};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("Hello, World", ss.str());
}
#endif

TEST(integer)
{
    auto              i = 0x7ff;
    std::stringstream ss;
    ss << UnitTests::stream_any(i);
    ASSERT_EQUALS("2047", ss.str());
}

TEST(character)
{
    auto              i = 'c';
    std::stringstream ss;
    ss << UnitTests::stream_any(i);
    ASSERT_EQUALS("c", ss.str());
}

TEST(signed_character)
{
    auto              i = static_cast<signed char>('a');
    std::stringstream ss;
    ss << UnitTests::stream_any(i);
    ASSERT_EQUALS("97", ss.str());
}

TEST(unsigned_character)
{
    auto              i = static_cast<unsigned char>('a');
    std::stringstream ss;
    ss << UnitTests::stream_any(i);
    ASSERT_EQUALS("0x61", ss.str());
}

TEST(unsigned_long)
{
    auto              i = 0xabcdef12UL;
    std::stringstream ss;
    ss << UnitTests::stream_any(i);
    auto size{sizeof i};
    ASSERT_EQUALS((size == 8 ? "0x00000000abcdef12" : "0xabcdef12"), ss.str());
}

TEST(signed_long)
{
    auto              i = 0xabcdef12L;
    std::stringstream ss;
    ss << UnitTests::stream_any(i);
    auto size{sizeof i};
    ASSERT_EQUALS("2882400018", ss.str());
}