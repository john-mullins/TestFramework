#if !defined(TestFramework_StreamForTestOutput_h_)
#define TestFramework_StreamForTestOutput_h_

#include <ostream>

namespace UnitTests
{
	using true_type = std::integral_constant<bool, true>;
	using false_type = std::integral_constant<bool, false>;

	inline char escape_it(char c)
	{
		switch (c)
		{
		case '\n':	return 'n';
		case '\t':	return 't';
		case '\r':	return 'r';
		case '\v':	return 'v';
		default:	return c;
		}
	}

	void add_escapes(std::string& s, const char * escapes = "\n\t\r\v\\")
	{
		for (auto i = s.find_first_of(escapes); i != std::string::npos; i = s.find_first_of(escapes, i))
		{
			char c = s[i];
			s[i++] = '\\';
			s.insert(i++, 1, escape_it(c));
		}
	}

	inline char unescape_it(int c)
	{
		switch (c)
		{
		case 'n':	return '\n';
		case 't':	return '\t';
		case 'r':	return '\r';
		case 'v':	return '\v';
		default:	return static_cast<char>(c);
		}
	}

	template<class String>
	void remove_escapes(String& s, const char * escapes = "\n\t\r\v\\")
	{
		for (auto i = s.find_first_of(escapes); i != std::string::npos; i = s.find_first_of(escapes, i))
		{
			s.erase(i, 1);
			s[i] = unescape_it(s[i]);
			++i;
		}
	}

	namespace details
	{
		template<class T> struct expected_got_outputter;
		template<class T, class U> struct expected_got_outputter_with_coercion;
	}

	template<typename T, typename U>
		details::expected_got_outputter_with_coercion<T, U> stream_with_coercion(const T& t, const U& u)
	{
		return details::expected_got_outputter_with_coercion<T, U>(t, u);
	}

	template<typename T>
		details::expected_got_outputter<T> stream(const T& t)
	{
		return details::expected_got_outputter<T>(t);
	}

	// this bit of stream magic escapes strings, this is really useful for comparing strings with newlines etc in.
	namespace details
	{
		template<class T>
			struct expected_got_outputter
		{
			expected_got_outputter(const T& t) : t(t) {}
			const T& t;
		};
                
		template<class T>
			std::ostream& operator<<(std::ostream& os, const expected_got_outputter<T> & t)
		{	
				return os << t.t; // img::stream_any(t.t);
		}

		inline std::ostream& operator<<(std::ostream& os, const expected_got_outputter<std::string> & t)
		{	
			std::string c = t.t;
			add_escapes(c);
			return os << c;
		}
		template<class T, class U>
			struct expected_got_outputter_with_coercion
		{
			expected_got_outputter_with_coercion(const T& t, const U& u) : t(t), u(u) {}
			const T& t;
			const U& u;
		};

		template<class T, class U>
			std::ostream& streamit(std::ostream& s, const expected_got_outputter_with_coercion<T, U>& t, false_type)
		{ 
			return s << stream(t.t) << " (uncoerced=" << stream(t.u) << ")";
		}
		template<class T, class U>
			std::ostream& streamit(std::ostream& s, const expected_got_outputter_with_coercion<T, U>& t, true_type)
		{ 
			return s << stream(t.t);
		}

		template<class T, class U>
			inline std::ostream& operator<<(std::ostream& os, const expected_got_outputter_with_coercion<T, U> & t)
		{	
			return streamit(os, t, typename std::is_same<T, U>::type());
		}
	}
}



#endif



