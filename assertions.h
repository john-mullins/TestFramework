#if !defined(TestFramework_Assertions_h_)
#define TestFramework_Assertions_h_

#include "testfailure.h"
#include "streamfortestoutput.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <vector>

// use these macros to define your test conditions (they do as they say on the tin) :

#define ASSERT_EQUALS               UnitTests::Assert(__FILE__, __LINE__).Equals
#define ASSERT_NOT_EQUALS           UnitTests::Assert(__FILE__, __LINE__).NotEquals
#define ASSERT_TRUE                 UnitTests::Assert(__FILE__, __LINE__).True
#define ASSERT_FALSE                UnitTests::Assert(__FILE__, __LINE__).False
#define ASSERT_IN                   UnitTests::Assert(__FILE__, __LINE__).In
#define ASSERT_NOT_IN               UnitTests::Assert(__FILE__, __LINE__).NotIn
#define FAIL                        UnitTests::Assert(__FILE__, __LINE__).Fail
//#define ASSERT_FILES_EQUAL            UnitTests::Assert(__FILE__, __LINE__).FilesEqual
//#define ASSERT_LARGE_STRING_EQUALS    UnitTests::Assert(__FILE__, __LINE__).LargeStringEquals
#define ASSERT_RANGE_EQUALS         UnitTests::Assert(__FILE__, __LINE__).RangeEquals

// Note this isn't a very good implementation of this because it ignores trailing whitespace on newlines, but it is a start.
// compare multiline strings making it easier to spot differences when they are different
#define ASSERT_MULTI_LINE_EQUALS    UnitTests::Assert(__FILE__, __LINE__).MultiLineEquals

// use ASSERT_THROWS and ASSERT_THROWS_MSG to assert that code should test that code
// e.g.
//
//	void f(int n) 
//  { 
//    if (n < 0) throw std::out_of_range();
//	}
//
//	TEST(test_f)
//	{
//		ASSERT_THROWS(std::out_of_range, f(-1));
//	}
#define ASSERT_THROWS_MSG(msg, exception, code)                             \
	ASSERT_THROWS_WITH_MESSAGE_MSG(msg, exception, "", code);               \
/**/

#define ASSERT_THROWS(exception, code)                                      \
    ASSERT_THROWS_MSG(#exception " exception should have been thrown by "   \
                        #code, exception, code)                             \
/**/

// Ensure that eval(code) raises `exception`, and exception.what() should contain the string expectedmsg.
// For example:
//		std::vector<int> v;
//		ASSERT_THROWS_WITH_MESSAGE(std::index_error, "out of bounds", v.at(0));
#define ASSERT_THROWS_WITH_MESSAGE_MSG(msg, exception, expectedmsg, code)	\
    try                                                                     \
    {                                                                       \
      code;                                                                 \
      FAIL(msg);                                                            \
    }                                                                       \
    catch (const UnitTests::TestFailure&)	{ throw; }                      \
    catch (const exception& e)                                              \
    {                                                                       \
      std::string what = e.what();                                          \
      if (what.find(expectedmsg) == std::string::npos)                      \
      {                                                                     \
          FAIL(msg + std::string("\n"                                       \
                                 "Exception " #exception " was raised but what() did not contain expected message\n"    \
                                 "Expected : <") + expectedmsg + std::string(">\n"                                      \
                                                                             "Actual   : <") + what + ">");             \
      }                                                                     \
    }                                                                       \
/**/

#define ASSERT_THROWS_WITH_MESSAGE(exception, expectedmsg, code)                            \
        ASSERT_THROWS_WITH_MESSAGE_MSG(#exception " exception should have been thrown by "  \
            #code, exception, expectedmsg, code)                                            \
/**/

// use ASSERT_NO_THROW and ASSERT_NO_THROW_MSG to assert that the code does not throw
// e.g.
//
//	void f(int n) 
//  { 
//    if (n < 0) throw std::out_of_range();
//	}
//
//	TEST(test_f)
//	{
//		ASSERT_NO_THROW(f(0));
//	}
#define ASSERT_NO_THROW_MSG(msg, code)                          \
    try                                                         \
    {                                                           \
       code;                                                    \
    }                                                           \
    catch (...)                                                 \
    {                                                           \
       throw UnitTests::TestFailure(msg, __FILE__, __LINE__);   \
    }                                                           \
/**/

#define ASSERT_NO_THROW(code) 	ASSERT_NO_THROW_MSG("Unexpected exception thrown by " #code, code)


#define SKIP()					throw UnitTests::TestSkipped()
#define UNIMPLEMENTED			FAIL("Test not yet implemented");

namespace UnitTests
{
    // this makes an array look like a container, for functions/algorithms that expect an array.
    // e.g.
    // std::string bits[] = { "a", "b", "c" };
    // std::string s = join(as_container(bits), "");
    // ASSERT_EQUALS("abc", s);
    template<typename T, size_t N>
    struct container
    {
        container(T (&a)[ N ]) : begin_(a), end_(a + N) {}
        typedef typename std::remove_const<T>::type value_type;
        typedef const T* const_iterator;
        const_iterator begin() const    { return begin_;    }
        const_iterator end() const      { return end_;      }
        size_t size() const             { return N;         }
        bool empty() const              { return N == 0;    }
        const_iterator begin_;
        const_iterator end_;
    };
    template<typename T, size_t N>
    container<T, N> as_container(T (&a)[ N ])
    {
        return container<T, N>(a);
    }
        
    template <class T>
    bool are_equal(const T& lhs, const T& rhs)
    {
        return lhs == rhs;
    }
    
    // helpers for ASSERT_RANGE_EQUALS
    template<class It>
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
    
    template<class It1, class It2>
    void OutputRange(std::ostream& os, It1 first1, It1 last1, It2 first2, It2 last2, It1 point_out)
    {
        size_t n = 0;
        while (first1 != last1 || first2 != last2)
        {
            if (n != 0)
            {
                os << "\n";
            }
            os << "\telement[" << n++ << "] = (";
            bool point_out_this_line = first1 == point_out;
            first1 = OutputElement(os, first1, last1);
            os << ",";
            first2 = OutputElement(os, first2, last2);
            os << (point_out_this_line ? ") <--------- HERE" : ")");
        }
    }
    
    class Assert
    {
    public:
        Assert(const char * file, int line)
        :	m_file(file),
        m_line(line)
        {
        }
        
        template<class T, class U>
        void Equals(const T& expected, const U& actual) const
        {
            Equals("", expected, actual);
        }
        
        template<class T, class U>
        void Equals(const std::string& msg, const T& expected, const U& actual) const
        {
            if (!are_equal(std::common_type_t<T, U>(expected), std::common_type_t<T, U>(actual)))
            {
                std::ostringstream s;
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
        
        template<class T, class U>
        void NotEquals(const T& expected, const U& actual) const
        {
            NotEquals("", expected, actual);
        }
        
        template<class T, class U>
        void NotEquals(const std::string& msg, const T& expected, const U& actual) const
        {
            if (are_equal(std::common_type_t<T, U>(expected), std::common_type_t<T, U>(actual)))
            {
                std::ostringstream s;
                if (!msg.empty())
                    s << stream(msg) << " ";
                
                s << std::boolalpha;
                s << "Wasn't expecting to get <" << stream(static_cast<T>(actual)) << ">";
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
        
        template<typename Value, typename Container>
        void In(const std::string& msg, Value value, Container container) const
        {
            bool present = std::find(container.begin(), container.end(), value) != container.end();
            if (!present)
            {
                ContainmentError(msg, "Expected container to contain", value, container.begin(), container.end());
            }
        }
        
        template<typename Value, typename ContainerValue, size_t ContainerN>
        void In(const std::string& msg, Value value, ContainerValue (&container)[ ContainerN]) const
        {
            In(msg, value, as_container(container));
        }
        
        template<typename Value, typename ContainerValue, size_t ContainerN>
        void In(Value value, ContainerValue (&container)[ ContainerN]) const
        {
            In(std::string(), value, as_container(container));
        }
        
        template<typename T, typename Container>
        void In(T t, Container container) const
        {
            In(std::string(), t, container);
        }
        void In(const char* needle, std::string haystack) const { return In(std::string(needle), haystack); }
        
        void In(std::string needle, const char* haystack) const { return In(needle, std::string(haystack)); }
        
        void In(std::string needle, std::string haystack) const
        {
            bool present = haystack.find(needle) != std::string::npos;
            if (!present)
            {
                std::stringstream s;
                s << "Expected to find \"" << needle << "\" in string \"" << haystack << " ";
                Error(s.str());
            }
        }
        
        void NotIn(const char* needle, std::string haystack) const { return NotIn(std::string(needle), haystack); }
        
        void NotIn(std::string needle, const char* haystack) const { return NotIn(needle, std::string(haystack)); }
        
        void NotIn(std::string needle, std::string haystack) const
        {
            bool present = haystack.find(needle) != std::string::npos;
            if (present)
            {
                std::stringstream s;
                s << "Did not expect to find \"" << needle << "\" in string \"" << haystack << " ";
                Error(s.str());
            }
        }
        
        template<typename Value, typename Container>
        void NotIn(const std::string& msg, Value value, Container container) const
        {
            bool present = std::find(container.begin(), container.end(), value) != container.end();
            if (present)
            {
                ContainmentError(msg, "Did not expect container to contain", value, container.begin(), container.end());
            }
        }
        
        template<typename Value, typename ContainerValue, size_t ContainerN>
        void NotIn(const std::string& msg, Value value, ContainerValue (&container)[ ContainerN]) const
        {
            NotIn(msg, value, as_container(container));
        }
        
        template<typename Value, typename ContainerValue, size_t ContainerN>
        void NotIn(Value value, ContainerValue (&container)[ ContainerN]) const
        {
            NotIn(std::string(), value, as_container(container));
        }
        
        template<typename T, typename Container>
        void NotIn(T t, Container container) const
        {
            NotIn(std::string(), t, container);
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
        
        void Fail() const
        {
            Fail("Test FAIL'ed");
        }
        
        void Fail(const std::string& msg) const
        {
            Error(msg);
        }
        
        void FilesEqual(const std::string& msg, const std::string& expected_filename, const std::string& actual_filename);
        
        void FilesEqual(const std::string& expected_filename, const std::string& actual_filename)
        {
            FilesEqual(std::string(), expected_filename, actual_filename);
        }
        
        void LargeStringEquals(const std::string& msg, const std::string& expected, const std::string& actual);
        
        void LargeStringEquals(const std::string& expected, const std::string& got)
        {
            LargeStringEquals(std::string(), expected, got);
        }
        
        template<class ExpectedIt, class GotIterator>
        void RangeEquals(const std::string& msg, ExpectedIt expected_first, ExpectedIt expected_last, GotIterator got_first, GotIterator got_last) const
        {
            ptrdiff_t expected_len = std::distance(expected_first, expected_last);
            ptrdiff_t got_len = std::distance(got_first, got_last);
            if (expected_len != got_len)
            {
                RangeError(msg, " length", expected_first, expected_last, got_first, got_last, expected_last, expected_len, got_len);
            }
            
            ExpectedIt dif = std::mismatch(expected_first, expected_last, got_first).first;
            if (dif != expected_last)
            {
                RangeError(msg, "", expected_first, expected_last, got_first, got_last, dif, expected_len, got_len);
            }
        }
        
        template<class ExpectedIt, class GotIterator>
        void RangeEquals(ExpectedIt expected_first, ExpectedIt expected_last, GotIterator got_first, GotIterator got_last) const
        {
            RangeEquals(std::string(), expected_first, expected_last, got_first, got_last);
        }
        
        template<class ExpectedRange, class GotRange>
        void RangeEquals(const std::string& msg, ExpectedRange expected, GotRange got) const
        {
            RangeEquals(msg, expected.begin(), expected.end(), got.begin(), got.end());
        }
        
        template<class ExpectedRange, class GotRange>
        void RangeEquals(ExpectedRange expected, GotRange got) const
        {
            RangeEquals(std::string(), expected, got);
        }
        
        template<typename ExpectedValue, size_t ExpectedN, class GotRange>
        void RangeEquals(ExpectedValue (&expected)[ ExpectedN ], GotRange got) const
        {
            RangeEquals(std::string(), std::vector<ExpectedValue>(expected), got);
        }
        
        template<class ExpectedRange, typename GotValue, size_t GotN>
        void RangeEquals(ExpectedRange expected, GotValue (&got)[ GotN ]) const
        {
            RangeEquals(std::string(), expected, as_container(got));
        }
        
        template<typename ExpectedValue, size_t ExpectedN, typename GotValue, size_t GotN>
        void RangeEquals(ExpectedValue (&expected)[ ExpectedN ], GotValue (&got)[ GotN ]) const
        {
            RangeEquals(std::string(), as_container(expected), as_container(got));
        }
        
        template<typename ExpectedValue, size_t ExpectedN, class GotRange>
        void RangeEquals(const std::string& msg, ExpectedValue (&expected)[ ExpectedN ], GotRange got) const
        {
            RangeEquals(msg, as_container(expected), got);
        }
        
        template<class ExpectedRange, typename GotValue, size_t GotN>
        void RangeEquals(const std::string& msg, ExpectedRange expected, GotValue (&got)[ GotN ]) const
        {
            RangeEquals(msg, expected, as_container(got));
        }
        
        template<typename ExpectedValue, size_t ExpectedN, typename GotValue, size_t GotN>
        void RangeEquals(const std::string& msg, ExpectedValue (&expected)[ ExpectedN ], GotValue (&got)[ GotN ]) const
        {
            RangeEquals(msg, as_container(expected), as_container(got));
        }

        inline std::string spacer(const std::string& s, size_t width, char fillchar)
        {
            return s.size() < width ? std::string(width - s.size(), fillchar) : std::string();
        }

        // like pythons functions of the same name (in module string and string methods)
        inline std::string ljust(const std::string& s, size_t width, char fillchar = ' ') { return s + spacer(s, width, fillchar); }
        inline std::string rjust(const std::string& s, size_t width, char fillchar = ' ') { return spacer(s, width, fillchar) + s; }
        
        // Note this isn't a very good implementation of this because it ignores trailing whitespace on newlines, but it is a start.
        void MultiLineEquals(std::string message, std::vector<std::string> expected, std::vector<std::string> got)
        {
            size_t width = std::max_element(expected.begin(), expected.end(),  [] (const std::string& s1, const std::string& s2) { return s1.size() < s2.size(); } )->size();
            std::vector<std::string> e, g;
            e.reserve(expected.size());
            g.reserve(got.size());
            std::transform(expected.begin(), expected.end(), std::back_inserter(e), [&] (const std::string& s1) {return ljust(s1, width, ' '); });
            std::transform(got.begin(),      got.end(),      std::back_inserter(g), [&] (const std::string& s1) {return ljust(s1, width, ' '); });
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
			MultiLineEquals(message, expected, split(got, "\n"));
		}

    private:
        void Error(const std::string& msg) const
        {
            throw TestFailure(msg, m_file, m_line);
        }
        
        inline std::vector<std::string> split(const std::string& s, const char* delims = " \t\r\n\v" )
        {
            std::vector<std::string> results;
            
            auto start = s.find_first_not_of(delims);
            
            while (start != std::string::npos)
            {
                auto end = s.find_first_of(delims, start);
                if (end != start)
                    results.emplace_back(s, start, end == std::string::npos ? std::string::npos : end - start);
                
                start = s.find_first_not_of(delims, end);
            }
            
            return results;
        }
        
        template<class FwdIt>
        void join(FwdIt begin, FwdIt end, const std::string& delim, std::ostream& stream) const
        {
            if (begin != end)
            {
                stream << *begin;
                for (++begin; begin != end; ++begin)
                {
                    stream << delim << *begin;
                }
            }
        }
        
        template<class FwdIt>
        std::string join(FwdIt begin, FwdIt end, const std::string& delim) const
        {
            std::ostringstream	stream;
            join(begin, end, delim, stream);
            return stream.str();
        }
        
        template<class Range>
        std::string join(Range range, const std::string& delim) const
        {
            return join(range.begin(), range.end(), delim);
        }
        
        template<typename Value, typename ContainerIterator>
        void ContainmentError(const std::string& msg, const std::string& msg2, Value value, ContainerIterator begin, ContainerIterator end) const
        {
            std::ostringstream s;
            s << stream(msg);
            if (!msg.empty())
            {
                s << ". ";
            }
            s << msg2 << " ";
            s << stream(value);
            s << ", actual contents :\n\t";
            join(begin, end, "\n\t", s);
            s << "\n";
            Error(s.str());
        }
        
        template<class ExpectedIt, class GotIterator>
        void RangeError(const std::string& msg, const std::string& reason,
                        ExpectedIt expected_first, ExpectedIt expected_last, GotIterator got_first, GotIterator got_last,
                        ExpectedIt indicate, ptrdiff_t expected_len, ptrdiff_t got_len) const
        {
            std::ostringstream s;
            if (!msg.empty())
            {
                s << stream(msg) << " ";
            }
            s << "Expected range [" << expected_len << "] different" << reason << " to actual range [" << got_len << "]\n";
            OutputRange(s, expected_first, expected_last, got_first, got_last, indicate);
            Error(s.str());
        }
        const char *	m_file;
        int				m_line;
    };
}

#endif

