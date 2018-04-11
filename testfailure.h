#if !defined(TestFailure_h_)
#define TestFailure_h_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <exception>
#include <string>
//#include "imgleeds/imgleeds/to_string.h"

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
			:	m_what(FormatError(file, line, AssertionFailure) + "Assertion failure : " + msg),
				m_msg(msg)
		{
		}

		const char* what() const throw() { return m_what.c_str(); }

		// get the failure message WITHOUT the file and line number bit
		std::string msg() const { return m_msg; }

		void AddStackTrace(const char * file, const char * function, int line)
		{
			m_what = what();
			m_what += "\n\t";
			m_what += file;
			m_what += "(" + std::to_string(line) + ") : called from ";
			m_what += (function[0] != 0 ? function : "(unknown)");
			m_what += ".";
		}

		~TestFailure() throw() {}
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

		const char* what() const throw() { return m_what.c_str(); }

		// get the failure message WITHOUT the file and line number bit
		std::string msg() const { return m_msg; }

		~TestTimeout() throw() {}
	private:
		std::string	m_what;
		std::string m_msg;
	};


	class TestSkipped : public std::exception
	{
	};

}

#endif

