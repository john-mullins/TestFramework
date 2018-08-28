#if !defined(TestFailure_h_)
#define TestFailure_h_
#include <exception>
#include <string>

namespace UnitTests
{
    std::string FormatError(const std::string& file, int line, int error);
    std::string FormatError(const std::string& file, const std::string& line, int error);

    enum
    {
        AssertionFailure = 1000,
        TimeoutFailure,
        UnexpectedException,
        UnknownUnexpectedException
    };

    class TestFailure : public std::exception
    {
    public:
        TestFailure(
            const std::string& msg, const std::string& file, int line, const std::string& failure_type, int error_code)
            : m_what(FormatError(file, line, error_code) + failure_type + msg), m_msg(msg)
        {
        }

        TestFailure(const std::string& msg, const std::string& file, int line)
            : TestFailure(msg, file, line, "Assertion failure : ", AssertionFailure)
        {
        }

        const char* what() const noexcept override
        {
            return m_what.c_str();
        }

        // get the failure message WITHOUT the file and line number bit
        std::string msg() const
        {
            return m_msg;
        }

    private:
        std::string m_what;
        std::string m_msg;
    };

    class TestTimeout : public TestFailure
    {
    public:
        TestTimeout(const std::string& msg, const std::string& file, int line)
            : TestFailure(msg, file, line, "Test timeout : ", TimeoutFailure)
        {
        }
    };

    class TestSkipped : public std::exception
    {
    };

} // namespace UnitTests

#endif
