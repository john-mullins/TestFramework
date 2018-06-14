#if !defined(TestFailure_h_)
#define TestFailure_h_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
        TestFailure(const std::string& msg, const std::string& file, int line)
        :   m_what(FormatError(file, line, AssertionFailure) + "Assertion failure : " + msg),
            m_msg(msg)
        {
        }
        
        const char* what() const noexcept { return m_what.c_str(); }
        
        // get the failure message WITHOUT the file and line number bit
        std::string msg() const { return m_msg; }
        
        ~TestFailure() noexcept {}
    private:
        std::string	m_what;
        std::string m_msg;
    };
    
    class TestTimeout: public std::exception
    {
    public:
        TestTimeout(const std::string& msg, const std::string& file, int line)
        :	m_what(FormatError(file, line, TimeoutFailure) + "Test timeout : " + msg),
            m_msg(msg)
        {
        }
        
        const char* what() const noexcept { return m_what.c_str(); }
        
        // get the failure message WITHOUT the file and line number bit
        std::string msg() const { return m_msg; }
        
        ~TestTimeout() noexcept {}
    private:
        std::string	m_what;
        std::string m_msg;
    };
    
    
    class TestSkipped : public std::exception
    {
    };
    
}

#endif

