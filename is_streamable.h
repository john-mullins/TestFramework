
#if !defined(is_streamable_h)
#define is_streamable_h

#include <iosfwd>
#include <type_traits>

// Provides a traits class is_streamable<T> which detects at compile time whether a type has a suitable 
// overload for std::ostream operator<<(std::ostream&, T); 

namespace UnitTests
{
    typedef char (&no_tag)[1];
    typedef char (&yes_tag)[2];
    
    namespace is_streamable_details
    {
        // this type has a UDC from anything, and will allow our own operator<< (declared below)
        // to be a match if no operator<< matches better.  However the ... makes this a very 
        // low priority match, ie the last resort for the compiler
        struct non_streamable_type
        {
            non_streamable_type(...);
#if defined(__GNUC__)
            // the above function makes is_streamable fail to compile for non streamable UDT because
            // passing UDT through ... is non-conforming.
            // this template overload is also a very low prriority match for the compiler, but higher than ...
            template<typename Other>
            non_streamable_type(Other);
#endif
        };
        
        // this function will be the result of the overload resolution of std::ostream& << const T& if 
        // no better overload exists (e.g an actual operator<<)
        struct non_streamable_result {};
        template<typename T>
        non_streamable_result operator<<(const T& t, const non_streamable_type & u);
        
        // used in sizeof expression to generate types to pass to <<
        template<typename T>
        T create_a();
        
        // these allow us to test whether our own operator << was matched, or another one from somewhere else.
        no_tag	is_streamable_result(const non_streamable_result&);
#if defined(__GNUC__)
        // without this overload on gcc (2.95 - possibly others) we get a warning about passing ostream through ...
        yes_tag	is_streamable_result(std::ostream&);
#endif
        yes_tag	is_streamable_result(...);
        
        template<typename T>
        struct is
        {
            // Does the actual test, essentially tests the return value of Stream << Type, and works out which 
            // overload was called.  This has to be in this namespace, or else << is not looked up correctly.
            static constexpr bool value = sizeof(is_streamable_result(create_a<std::ostream&>() << create_a<const T&>())) == sizeof(yes_tag);
        };
    }
    
    // the interface, we could probably use a using declaration, but that would probably break the name lookup rules
    // on some compilers.
    //	Examples : 
    //
    //		The simplest case, use the ::value : 
    //
    //		template<class T>
    //			void display_streamable(const T & t)
    //		{
    //			if (img::is_streamable<T>::value)
    //			{
    //				std::cout << "Yes";
    //				// std::cout << t;   CAN'T DO THIS.
    //			}
    //			else
    //			{
    //				std::cout << "No";
    //			}
    //		}
    //
    // The problem with this is that you can't actually call << because it would fail to compile for types that are not, instead use ::type : 
    //
    //		template<class T>
    //			void display_streamable(const T& t, std::true_type&)
    //		{
    //			std::cout << t;
    //		}
    //
    //		template<class T>
    //			void display_streamable(const T&, std::false_type_&)
    //		{
    //			std::cout << "<nonstreamable>";
    //		}
    //
    //		template<class T>
    //			void display_streamable(const T & t)
    //		{
    //			display_streamable(t, img::is_streamable<T>::type());
    //		}
    //		
    
    template<typename T>
    struct is_streamable : std::integral_constant<bool, is_streamable_details::is<T>::value>
    {
    };
}

#endif

