#if !defined(streamable_h)
#define streamable_h

#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <deque>
#include <utility>

using std::cbegin;
using std::cend;

namespace UnitTests
{
    // Provides a traits class is_streamable<T> which detects at compile time whether a type has a suitable
    // overload for std::ostream operator<<(std::ostream&, T);
    //    Examples :
    //
    //        The simplest case, use the ::value :
    //
    //        template<class T>
    //            void display_streamable(const T & t)
    //        {
    //            if (is_streamable<T>::value)
    //            {
    //                std::cout << "Yes";
    //                // std::cout << t;   CAN'T DO THIS.
    //            }
    //            else
    //            {
    //                std::cout << "No";
    //            }
    //        }
    //
    // The problem with this is that you can't actually call << because it would fail to compile for types that are not, instead use ::type :
    //
    //        template<class T>
    //            void display_streamable(const T& t, std::true_type&)
    //        {
    //            std::cout << t;
    //        }
    //
    //        template<class T>
    //            void display_streamable(const T&, std::false_type_&)
    //        {
    //            std::cout << "<nonstreamable>";
    //        }
    //
    //        template<class T>
    //            void display_streamable(const T & t)
    //        {
    //            display_streamable(t, is_streamable<T>());
    //        }
    //
    //      In C++17 we will be able to use constexpr if like this:
    //
    //      template<class T>
    //          void display_streamable(const T& t)
    //      {
    //          if constexpr (is_streamable<T>())
    //              std::cout << t;
    //          else
    //              std::cout << "<nonstreamable>";
    //      }
    //
    namespace details
    {
        //  Need this as std::void_t is C++ 17
        template<typename...>
        using void_t = void;
    }
    
    template<typename T, typename = void>
    struct is_streamable : std::false_type {};
    
    template<typename T>
    struct is_streamable<T, details::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};

    
    namespace stream_any_details
    {
        // this should really be localised somehow, but this will have to do for now.
        inline const char * get_nonstreamable_output()
        {
            return "nonstreamable";
        }
        
        // ---- the streaming functions : -----
        
        //for nonstreamable types
        template<typename T>
        void output(std::ostream& s, const T&, const std::false_type&)
        {
            s << get_nonstreamable_output();
        }

        // unsigned types are much nicer in hex :
        template<class T>
        void output_unsigned(std::ostream& s, const T & t)
        {
            s << std::hex << std::showbase << t << std::noshowbase << std::dec;
        }

        //  signed and unsigned chars are numeric types not characters
        inline void output_unsigned(std::ostream& s, unsigned char t)
        {
            output_unsigned(s, static_cast<unsigned int>(t));
        }

        inline void output(std::ostream& s, signed char t, const std::true_type&)
        {
            s << static_cast<signed int>(t);
        }

        
        // the default streamer
        template<class T>
        static void output(std::ostream& s, const T & t, const std::true_type&)
        {
            //  The optimiser will probably throw away the non taken branch
            //  In C++17 if constexpr would gaurantee that only the taken
            //  branch is compiled
            if (std::is_unsigned<T>::value)
                output_unsigned(s, t);
            else
                s << t;
        }
        
        // the placeholder object, we create one of these with the 'output' function in the parent namespace.
        template<class T>
        struct outputter
        {
            outputter(const T& t) : t(t) {}
            const T& t;
        };
    }
    
    template<class T>
    typename stream_any_details::outputter<T> stream_any(const T& t);
    
    namespace stream_any_details
    {
        // here provide some overloads for some fairly common types that we can introspect, and stream_any a bit deeper.
        
        //  Here we have a pair of functions that implement std::apply
        //  apply is a C++17 function, these work in C++14
        template <typename Function, typename Tuple, std::size_t... I>
        constexpr decltype(auto) apply(Function&& f, Tuple&& t, std::index_sequence<I...>)
        {
            return f(std::get<I>(std::forward<Tuple>(t))...);
        }
        
        template <typename Function, typename Tuple>
        constexpr decltype(auto) apply(Function&& f, Tuple&& t)
        {
            return apply(std::forward<Function>(f), std::forward<Tuple>(t),
                              std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value> {} );
        }
        
        //  Tuple support
        template <typename Head>
        constexpr void output_tuple(std::ostream& s, const Head& t)
        {
            s << stream_any(t);
        }

        template <typename Head, typename... Tail>
        constexpr void output_tuple(std::ostream& s, const Head& head, Tail... tail)
        {
            s << stream_any(head) << ", ";
            output_tuple(s, std::forward<Tail>(tail)...);
        }
        
        template <template <class ...> class tuple, typename... Ts>
        constexpr void output_container(std::ostream& s, const tuple<Ts...>& tup)
        {
            s << "(";
            auto f = [&s](auto... tail) { output_tuple(s, std::forward<Ts>(tail)... ); };
            apply(f, tup);
            s << ")";
        }
        
        template<typename... Ts>
        constexpr void output(std::ostream& s, const std::tuple<Ts...>& tup, const std::false_type&)
        {
            output_container(s, tup);
        }

        // Pair support
        template<typename First, typename Second>
        constexpr void output(std::ostream& s, const std::pair<First, Second>& tup, const std::false_type&)
        {
            output_container(s, tup);
        }

        template<typename FwdIt>
        void output_range(std::ostream& s, FwdIt begin, FwdIt end)
        {
            s << "[\n";
            for (; begin != end; ++begin)
            {
                s << "  " << stream_any(*begin) << ",\n";
            }
            s << "]\n";
        }

       // containers support.
        template<typename Type>
        void output(std::ostream& s, const std::vector<Type>& c, const std::false_type&)
        {
            output_range(s, cbegin(c), cend(c));
        }
        
        template<typename Type>
        void output(std::ostream& s, const std::list<Type>& c, const std::false_type&)
        {
            output_range(s, cbegin(c), cend(c));
        }
        
        template<typename KeyType, typename ValueType>
        void output(std::ostream& s, const std::map<KeyType, ValueType>& c, const std::false_type&)
        {
            output_range(s, cbegin(c), cend(c));
        }
        
        template<typename KeyType, typename ValueType>
        void output(std::ostream& s, const std::multimap<KeyType, ValueType>& c, const std::false_type&)
        {
            output_range(s, cbegin(c), cend(c));
        }
        
        template<typename Type>
        void output(std::ostream& s, const std::set<Type>& c, const std::false_type&)
        {
            output_range(s, cbegin(c), cend(c));
        }
        
        template<typename Type>
        void output(std::ostream& s, const std::multiset<Type>& c, const std::false_type&)
        {
            output_range(s, cbegin(c), cend(c));
        }
        
        template<typename Type>
        void output(std::ostream& s, const std::deque<Type>& c, const std::false_type&)
        {
            output_range(s, cbegin(c), cend(c));
        }
        
        // add support for C-arrays,
        // (note that is_streamable returns true, because there is a valid conversion from array to pointer type)
        template <typename T, size_t N >
        void output(std::ostream& s,  T (&a)[ N ], const std::true_type&)
        {
            output_range(s, cbegin(a), cend(a));
        }
        
        template <typename T, size_t N >
        void output(std::ostream& s,  const T (&a)[ N ], const std::true_type&)
        {
            output_range(s, cbegin(a), cend(a));
        }
        
        // the above template was a better match for char arrays (ie strings), so overload it AGAIN!
        template <size_t N >
        void output(std::ostream& s,  const char (&str)[ N ], const std::true_type&)
        {
            s << str;
        }
        
        template<class T>
        std::ostream& operator<<(std::ostream& s, const outputter<T> & t)
        {	// this call here will dispatch to a function that streams or not depending on whether
            // T has a suitable operator<<.
            output(s, t.t, is_streamable<T>());
            return s;
        }
    }
    
    // the manipulator creation function - ie the interface to all of the above
    // to use :
    //     std::cout << stream_any(myobject);
    template<class T>
    typename stream_any_details::outputter<T> stream_any(const T& t)
    {
        return stream_any_details::outputter<T>(t);
    }
    
    // a useful helper function, a bit like lexical cast, but it will only convert to string, not to other types.
    template<class T>
    std::string any_to_string(const T& t)
    {
        std::ostringstream os;
        os << stream_any(t);
        return os.str();
    }
}

#endif


