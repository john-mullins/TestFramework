#if !defined(TestFramework_Assertions_h_)
#define TestFramework_Assertions_h_

#include "streamfortestoutput.h"
#include "testfailure.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <sstream>
#include <vector>

using std::begin;
using std::end;

// use these macros to define your test conditions (they do as they say on the tin) :

#define ASSERT_EQUALS UnitTests::Assert(__FILE__, __LINE__).Equals
#define ASSERT_NOT_EQUALS UnitTests::Assert(__FILE__, __LINE__).NotEquals
#define ASSERT_TRUE UnitTests::Assert(__FILE__, __LINE__).True
#define ASSERT_FALSE UnitTests::Assert(__FILE__, __LINE__).False
#define ASSERT_IN UnitTests::Assert(__FILE__, __LINE__).In
#define ASSERT_NOT_IN UnitTests::Assert(__FILE__, __LINE__).NotIn
#define FAIL UnitTests::Assert(__FILE__, __LINE__).Fail
#define ASSERT_RANGE_EQUALS UnitTests::Assert(__FILE__, __LINE__).RangeEquals

// Note this isn't a very good implementation of this because it ignores trailing whitespace on newlines, but it is a
// start. compare multiline strings making it easier to spot differences when they are different
#define ASSERT_MULTI_LINE_EQUALS UnitTests::Assert(__FILE__, __LINE__).MultiLineEquals

// use ASSERT_THROWS and ASSERT_THROWS_MSG to assert that code should test that code
// e.g.
//
//  void f(int n)
//  {
//      if (n < 0) throw std::out_of_range();
//  }
//
//  TEST(test_f)
//  {
//      ASSERT_THROWS(std::out_of_range, f(-1));
//  }
#define ASSERT_THROWS_MSG(msg, exception, code)               \
    ASSERT_THROWS_WITH_MESSAGE_MSG(msg, exception, "", code); \
    /**/

#define ASSERT_THROWS(exception, code)                                                            \
    ASSERT_THROWS_MSG(#exception " exception should have been thrown by " #code, exception, code) \
/**/

// Ensure that eval(code) raises `exception`, and exception.what() should contain the string expectedmsg.
// For example:
//      std::vector<int> v;
//      ASSERT_THROWS_WITH_MESSAGE(std::index_error, "out of bounds", v.at(0));
#define ASSERT_THROWS_WITH_MESSAGE_MSG(msg, exception, expectedmsg, code)                            \
    try                                                                                              \
    {                                                                                                \
        code;                                                                                        \
        FAIL(msg);                                                                                   \
    }                                                                                                \
    catch (const UnitTests::TestFailure&)                                                            \
    {                                                                                                \
        throw;                                                                                       \
    }                                                                                                \
    catch (const exception& e)                                                                       \
    {                                                                                                \
        using namespace std::literals;                                                               \
        auto what = std::string{e.what()};                                                           \
        if (what.find(expectedmsg) == std::string::npos)                                             \
        {                                                                                            \
            FAIL((msg) +                                                                             \
                 "\n"                                                                                \
                 "Exception " #exception " was raised but what() did not contain expected message\n" \
                 "Expected : <"s +                                                                   \
                 (expectedmsg) +                                                                     \
                 ">\n"                                                                               \
                 "Actual   : <"s +                                                                   \
                 what + ">"s);                                                                       \
        }                                                                                            \
    }                                                                                                \
    /**/

#define ASSERT_THROWS_WITH_MESSAGE(exception, expectedmsg, code)                                 \
    ASSERT_THROWS_WITH_MESSAGE_MSG(                                                              \
        #exception " exception should have been thrown by " #code, exception, expectedmsg, code) \
/**/

// use ASSERT_NO_THROW and ASSERT_NO_THROW_MSG to assert that the code does not throw
// e.g.
//
//  void f(int n)
//  {
//      if (n < 0) throw std::out_of_range();
//  }
//
//  TEST(test_f)
//  {
//      ASSERT_NO_THROW(f(0));
//  }
#define ASSERT_NO_THROW_MSG(msg, code)                         \
    try                                                        \
    {                                                          \
        code;                                                  \
    }                                                          \
    catch (...)                                                \
    {                                                          \
        throw UnitTests::TestFailure(msg, __FILE__, __LINE__); \
    }                                                          \
    /**/

#define ASSERT_NO_THROW(code) ASSERT_NO_THROW_MSG("Unexpected exception thrown by " #code, code)

#define SKIP() throw UnitTests::TestSkipped()
#define UNIMPLEMENTED FAIL("Test not yet implemented");

namespace UnitTests
{
    template <class T>
    bool are_equal(const T& lhs, const T& rhs)
    {
        return lhs == rhs;
    }

    // helpers for ASSERT_RANGE_EQUALS
    template <class It>
    It OutputElement(std::ostream& os, It iter, It end)
    {
        if (iter != end)
        {
            os << stream(*iter);
            return ++iter;
        }
        os << "##EOF!##";
        return iter;
    }

    template <class It1, class It2>
    void OutputRange(std::ostream& os, It1 first1, It1 last1, It2 first2, It2 last2, It1 point_out)
    {
        auto n = 0U;
        while (first1 != last1 || first2 != last2)
        {
            os << "\telement[" << n++ << "] = (";
            auto point_out_this_line = first1 == point_out;

            first1 = OutputElement(os, first1, last1);
            os << ",";
            first2 = OutputElement(os, first2, last2);
            os << (point_out_this_line ? ") <--------- HERE\n" : ")\n");
        }
    }

    class Assert
    {
    public:
        Assert(const char* file, int line) : m_file(file), m_line(line)
        {
        }

        template <class T, class U>
        void Equals(const T& expected, const U& actual) const
        {
            Equals(std::string(), expected, actual);
        }

        template <class T, class U>
        void Equals(const std::string& msg, const T& expected, const U& actual) const
        {
            if (!are_equal(std::common_type_t<T, U>(expected), std::common_type_t<T, U>(actual)))
            {
                auto s = std::ostringstream{};
                if (!msg.empty())
                    s << stream(msg) << " ";

                s << std::boolalpha << "\n";
                s << "    Expected <" << stream_with_coercion(std::common_type_t<T, U>(expected), expected) << ">\n";
                s << "     but got <" << stream_with_coercion(std::common_type_t<T, U>(actual), actual) << ">\n";
                s << "  ";
                Error(s.str());
            }
        }

        void Equals(const char* msg, const char* expected, const std::string& actual) const
        {
            Equals(msg, std::string(expected), actual);
        }

        template <class T, class U>
        void NotEquals(const T& expected, const U& actual) const
        {
            NotEquals(std::string(), expected, actual);
        }

        template <class T, class U>
        void NotEquals(const std::string& msg, const T& expected, const U& actual) const
        {
            if (are_equal(std::common_type_t<T, U>(expected), std::common_type_t<T, U>(actual)))
            {
                auto s = std::ostringstream{};
                if (!msg.empty())
                    s << stream(msg) << " ";

                s << std::boolalpha;
                s << "Wasn't expecting to get <" << stream(std::common_type_t<T, U>(actual)) << ">";
                Error(s.str());
            }
        }

        void NotEquals(const char* msg, const char* expected, const std::string& actual) const
        {
            NotEquals(msg, std::string(expected), actual);
        }

        void True(bool expr) const
        {
            True("", expr);
        }

        void True(const std::string& msg, bool expr) const
        {
            if (!expr)
            {
                Error(msg + " Expression evaluated to false");
            }
        }

        [[noreturn]] void CreateInError(const char* msg, const std::string& needle, const std::string& haystack) const {
            auto s = std::stringstream{};
            s << msg << needle << "\" in string \"" << haystack << " ";
            Error(s.str());
        }

        template <typename Value, typename Container>
        void In(const std::string& msg, Value value, Container&& container) const
        {
            if (std::find(begin(container), end(container), value) == cend(container))
            {
                ContainmentError(msg, "Expected container to contain", value, begin(container), end(container));
            }
        }

        template <typename T, typename Container>
        void In(T t, Container&& container) const
        {
            In(std::string(), t, std::forward<Container>(container));
        }

        void In(const char* needle, const std::string& haystack) const
        {
            return In(std::string(needle), haystack);
        }

        void In(const char* needle, const char* haystack) const
        {
            return In(std::string(needle), std::string(haystack));
        }

        void In(const std::string& needle, const char* haystack) const
        {
            return In(needle, std::string(haystack));
        }

        void In(std::string needle, std::string haystack) const
        {
            if (haystack.find(needle) == std::string::npos)
                CreateInError("Expected to find \"", needle, haystack);
        }

        void NotIn(const char* needle, const std::string& haystack) const
        {
            return NotIn(std::string(needle), haystack);
        }

        void NotIn(const std::string& needle, const char* haystack) const
        {
            return NotIn(needle, std::string(haystack));
        }

        void NotIn(const char* needle, const char* haystack) const
        {
            return NotIn(std::string(needle), std::string(haystack));
        }

        void NotIn(std::string needle, std::string haystack) const
        {
            if (haystack.find(needle) != std::string::npos)
                CreateInError("Did not expect to find \"", needle, haystack);
        }

        template <typename Value, typename Container>
        void NotIn(const std::string& msg, Value value, Container&& container) const
        {
            if (std::find(begin(container), end(container), value) != cend(container))
            {
                ContainmentError(msg, "Did not expect container to contain", value, begin(container), end(container));
            }
        }

        template <typename T, typename Container>
        void NotIn(T t, Container&& container) const
        {
            NotIn(std::string(), t, std::forward<Container>(container));
        }

        void False(bool expr) const
        {
            False("", expr);
        }

        void False(const std::string& msg, bool expr) const
        {
            if (expr)
            {
                Error(msg + " Expression evaluated to true");
            }
        }

        [[noreturn]] void Fail() const { Fail("Test FAIL'ed"); }

            [[noreturn]] void Fail(const std::string& msg) const
        {
            Error(msg);
        }

        void LargeStringEquals(const std::string& msg, const std::string& expected, const std::string& actual);

        void LargeStringEquals(const std::string& expected, const std::string& got)
        {
            LargeStringEquals(std::string(), expected, got);
        }

        template <class ExpectedIt, class GotIterator>
        void RangeEquals(const std::string& msg, ExpectedIt expected_first, ExpectedIt expected_last,
            GotIterator got_first, GotIterator got_last) const
        {
            auto expected_len = std::distance(expected_first, expected_last);
            auto got_len      = std::distance(got_first, got_last);
            if (expected_len != got_len)
            {
                RangeError(msg, " length", expected_first, expected_last, got_first, got_last, expected_last,
                    expected_len, got_len);
            }

            auto dif = std::mismatch(expected_first, expected_last, got_first).first;
            if (dif != expected_last)
            {
                RangeError(msg, "", expected_first, expected_last, got_first, got_last, dif, expected_len, got_len);
            }
        }

        template <class ExpectedIt, class GotIterator>
        void RangeEquals(
            ExpectedIt expected_first, ExpectedIt expected_last, GotIterator got_first, GotIterator got_last) const
        {
            RangeEquals(std::string(), expected_first, expected_last, got_first, got_last);
        }

        template <class ExpectedRange, class GotRange>
        void RangeEquals(const std::string& msg, ExpectedRange& expected, GotRange& got) const
        {
            RangeEquals(msg, begin(expected), end(expected), begin(got), end(got));
        }

        template <class ExpectedRange, class GotRange>
        void RangeEquals(ExpectedRange&& expected, GotRange&& got) const
        {
            RangeEquals(std::string(), begin(expected), end(expected), begin(got), end(got));
        }

        inline std::string spacer(const std::string& s, size_t width, char fillchar)
        {
            return s.size() < width ? std::string(width - s.size(), fillchar) : std::string();
        }

        // like pythons functions of the same name (in module string and string methods)
        inline std::string ljust(const std::string& s, size_t width, char fillchar = ' ')
        {
            return s + spacer(s, width, fillchar);
        }
        inline std::string rjust(const std::string& s, size_t width, char fillchar = ' ')
        {
            return spacer(s, width, fillchar) + s;
        }

        // Note this isn't a very good implementation of this because it ignores trailing whitespace on newlines, but it
        // is a start.
        void MultiLineEquals(std::string message, std::vector<std::string> expected, std::vector<std::string> got)
        {
            auto width = std::max_element(
                begin(expected), end(expected), [](const auto& s1, const auto& s2) { return s1.size() < s2.size(); })
                             ->size();
            auto e = std::vector<std::string>{};
            auto g = std::vector<std::string>{};
            e.reserve(expected.size());
            g.reserve(got.size());
            auto adjust = [&](const std::string& s1) { return ljust(s1, width, ' '); };
            std::transform(begin(expected), end(expected), std::back_inserter(e), adjust);
            std::transform(begin(got), end(got), std::back_inserter(g), adjust);
            RangeEquals(message, e, g);
        }

        void MultiLineEquals(std::vector<std::string> expected, std::vector<std::string> got)
        {
            MultiLineEquals(std::string(), expected, got);
        }

        void MultiLineEquals(std::vector<std::string> expected, std::string got)
        {
            MultiLineEquals(std::string(), expected, got);
        }

        void MultiLineEquals(std::string expected, std::vector<std::string> got)
        {
            MultiLineEquals(std::string(), expected, got);
        }

        void MultiLineEquals(std::string expected, std::string got)
        {
            MultiLineEquals(std::string(), expected, got);
        }

        void MultiLineEquals(std::string message, std::vector<std::string> expected, std::string got)
        {
            MultiLineEquals(message, expected, split(got, "\n"));
        }

        void MultiLineEquals(std::string message, std::string expected, std::vector<std::string> got)
        {
            MultiLineEquals(message, split(expected, "\n"), got);
        }

        void MultiLineEquals(std::string message, std::string expected, std::string got)
        {
            MultiLineEquals(message, split(expected, "`n"), split(got, "\n"));
        }

        template <typename iter>
        class Range
        {
        public:
            Range(iter begin, iter end) : m_begin(begin), m_end(end)
            {
            }

            iter begin() const
            {
                return m_begin;
            }
            iter end() const
            {
                return m_end;
            }

        private:
            iter m_begin;
            iter m_end;
        };

    private:
        [[noreturn]] void Error(const std::string& msg) const { throw TestFailure(msg, m_file, m_line); }

        inline std::vector<std::string> split(const std::string& s, const char* delims = " \t\r\n\v")
        {
            auto results = std::vector<std::string>{};

            auto start = s.find_first_not_of(delims);

            while (start != std::string::npos)
            {
                auto end = s.find_first_of(delims, start);
                if (end != start)
                    results.emplace_back(s.substr(start, end - start));

                start = s.find_first_not_of(delims, end);
            }

            return results;
        }

        template <typename Value, typename ContainerIterator>
        [[noreturn]] void ContainmentError(const std::string& msg, const std::string& msg2, Value value,
            ContainerIterator begin, ContainerIterator end) const {
            auto s = std::ostringstream{};
            if (!msg.empty())
                s << stream(msg) << ". ";

            s << msg2 << " ";
            s << stream(value);
            s << ", actual contents :\n\t";
            s << stream_any(Range(begin, end));
            s << "\n";
            Error(s.str());
        }

        template <class ExpectedIt, class GotIterator>
        [[noreturn]] void RangeError(const std::string& msg, const std::string& reason, ExpectedIt expected_first,
            ExpectedIt expected_last, GotIterator got_first, GotIterator got_last, ExpectedIt indicate,
            ptrdiff_t expected_len, ptrdiff_t got_len) const {
            auto s = std::ostringstream{};
            if (!msg.empty())
            {
                s << stream(msg) << " ";
            }
            s << "Expected range [" << expected_len << "] different" << reason << " to actual range [" << got_len
              << "]\n";
            OutputRange(s, expected_first, expected_last, got_first, got_last, indicate);
            Error(s.str());
        }

        const char* m_file;
        int m_line;
    };
} // namespace UnitTests

#endif
