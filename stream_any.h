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
#include "is_streamable.h"


using std::cbegin;
using std::cend;

namespace UnitTests
{
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
        
        // the default streamer
        template<class T>
        static void output(std::ostream& s, const T & t, const std::true_type&)
        {
            s << t;
        }
        
        // unsigned types are much nicer in hex :
        template<class T>
        void output_unsigned(std::ostream& s, const T & t)
        {
            s << std::hex << std::showbase << t << std::noshowbase << std::dec;
        }
        
        inline void output(std::ostream& s, unsigned char t, const std::true_type&)	    { output_unsigned(s, static_cast<unsigned int>(t));     }
        inline void output(std::ostream& s, unsigned short t, const std::true_type&)    { output_unsigned(s, t);                                }
        inline void output(std::ostream& s, unsigned int t, const std::true_type&)      { output_unsigned(s, t);                                }
        inline void output(std::ostream& s, unsigned long long t, const std::true_type&){ output_unsigned(s, t);                                }
        
        // here provide some overloads for some fairly common types that we can introspect, and stream_any a bit deeper.
        
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
        //  Here we have a pair of functions that implement std::apply
        //  apply is a C++17 function, these work in C++14
        template <typename Function, typename Tuple, std::size_t... I>
        constexpr decltype(auto) apply_impl(Function&& f, Tuple&& t, std::index_sequence<I...>)
        {
            return f(std::get<I>(std::forward<Tuple>(t))...);
        }
        
        template <typename Function, typename Tuple>
        constexpr decltype(auto) apply_tuple(Function&& f, Tuple&& t)
        {
            return apply_impl(std::forward<Function>(f), std::forward<Tuple>(t),
                              std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value> {} );
        }
        
        //  Tuple support
        template <typename Head>
        void output_tuple(std::ostream& s, const Head& t)
        {
            s << stream_any(t);
        }

        template <typename Head, typename... Tail>
        void output_tuple(std::ostream& s, const Head& head, Tail... tail)
        {
            s << stream_any(head) << ", ";
            output_tuple(s, std::forward<Tail>(tail)...);
        }

        template<typename... Ts>
        void output(std::ostream& s, const std::tuple<Ts...>& tup, const std::false_type&)
        {
            s << "(";
            auto f = [&s](auto... tail) { output_tuple(s, std::forward<Ts>(tail)... ); };
            apply_tuple(f, tup);
            s << ")";
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
    //   to use :
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


