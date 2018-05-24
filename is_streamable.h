
#if !defined(is_streamable_h)
#define is_streamable_h

#include <iosfwd>
#include <typeinfo>

// Provides a traits class is_streamable<T> which detects at compile time whether a type has a suitable 
// overload for std::ostream operator<<(std::ostream&, T); 

namespace UnitTests
{
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
    //			display_streamable(t, img::is_streamable<T>());
    //		}
    //		
    
    //  Need this as std::void_t is C++ 17
    template<typename...>
    using void_t = void;
    
    template<typename T, typename = void>
    struct is_streamable : std::false_type {};
    
    template<typename T>
    struct is_streamable<T, void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};
}

#endif

