#include "MiniTestFramework.h"
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

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
    const char        s[]{'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '\0'};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("Hello, World", ss.str());
}

#ifdef HAS_STRING_VIEW1
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
    auto size(sizeof i);
    ASSERT_EQUALS((size == 8 ? "0x00000000abcdef12" : "0xabcdef12"), ss.str());
}

TEST(signed_long)
{
    signed long       i = 0x2bcdef12L;
    std::stringstream ss;
    ss << UnitTests::stream_any(i);
    ASSERT_EQUALS("734916370", ss.str());
}

class NonStreamableClass
{
};

TEST(non_streamable)
{
    auto              s = NonStreamableClass{};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("nonstreamable", ss.str());
}

TEST(vector_int)
{
    auto              s = std::vector<int>{0, 1, 2, 156};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("[0, 1, 2, 156]", ss.str());
}

TEST(list_unsigned)
{
    auto              s = std::list<unsigned int>{0, 1, 2, 156};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("[0x00000000, 0x00000001, 0x00000002, 0x0000009c]", ss.str());
}

TEST(forward_list_string)
{
    auto              s = std::list<std::string>{"Klaatu", "Barada", "Nikto"};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("[Klaatu, Barada, Nikto]", ss.str());
}

TEST(deque_int)
{
    auto              s = std::deque<int>{0, 1, 2, 156};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("[0, 1, 2, 156]", ss.str());
}

TEST(set_string)
{
    auto              s = std::set<std::string>{"Klaatu", "Barada", "Nikto"};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("[Barada, Klaatu, Nikto]", ss.str());
}

TEST(map_int_string)
{
    auto              s = std::map<int, std::string>{{3, "Klaatu"}, {2, "Barada"}, {1, "Nikto"}};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("[(1, Nikto), (2, Barada), (3, Klaatu)]", ss.str());
}

TEST(std_array)
{
    auto              s = std::array<int, 4>{0, 1, 2, 156};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("[0, 1, 2, 156]", ss.str());
}

TEST(std_pair)
{
    auto              s = std::pair<int, std::string>{7, "Pieces of gold"s};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("(7, Pieces of gold)", ss.str());
}

TEST(std_tuple)
{
    auto              s = std::tuple<int, std::string, unsigned int>{7, "Pieces of gold"s, 257};
    std::stringstream ss;
    ss << UnitTests::stream_any(s);
    ASSERT_EQUALS("(7, Pieces of gold, 0x00000101)", ss.str());
}
