#if !defined(streamable_h)
#define streamable_h

#include <iomanip>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

namespace UnitTests
{
    // Provides a traits class is_streamable<T> which detects at compile time whether a type has a suitable
    // overload for std::ostream operator<<(std::ostream&, T);
    //    Examples :
    //
    //        The simplest case, use the ::value :
    //
    //        template<typename T>
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
    // The problem with this is that you can't actually call << because it would fail to compile for types that are not, instead
    // use tag dispatch :
    //
    //        template<tyepname T>
    //            void display_streamable(const T& t, std::true_type&)
    //        {
    //            std::cout << t;
    //        }
    //
    //        template<typename T>
    //            void display_streamable(const T&, std::false_type_&)
    //        {
    //            std::cout << "<nonstreamable>";
    //        }
    //
    //        template<typename T>
    //            void display_streamable(const T & t)
    //        {
    //            display_streamable(t, is_streamable<T>());
    //        }
    //
    //      In C++17 we will be able to use constexpr if like this:
    //
    //      template<typename T>
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

    //  If accessible functions begin and end exist which can be called for this type or it is an array then treat it as
    //  a container
    template<typename T, typename = void>
    struct is_container : std::is_array<T> {};
    
    using std::begin;
    using std::end;
    
    template<typename T>
    struct is_container<T, details::void_t<decltype(begin(std::declval<T>()), end(std::declval<T>()))>> : std::true_type {};
 
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
        template<typename T>
        void output_unsigned(std::ostream& s, const T& t, int width)
        {
            s << std::hex << std::showbase << std::setw(2 + width) << std::internal << std::setfill('0') << t <<
                std::noshowbase << std::dec;
        }

        //  signed and unsigned chars are numeric types not characters
        inline void output_unsigned(std::ostream& s, unsigned char t, int width)
        {
            output_unsigned(s, static_cast<unsigned int>(t), width);
        }

        inline void output(std::ostream& s, signed char t, const std::true_type&)
        {
            s << static_cast<signed int>(t);
        }

        // the default streamer
        template<typename T>
        static void output(std::ostream& s, const T& t, const std::true_type&)
        {
            //  The optimiser will probably throw away the non taken branch
            //  In C++17 if constexpr would gaurantee that only the taken
            //  branch is compiled
            if (std::is_unsigned<T>::value)
                output_unsigned(s, t, 2 * sizeof t);
            else
                s << t;
        }
        
        // the placeholder object, we create one of these with the 'output' function in the parent namespace.
        template<typename T>
        struct outputter
        {
            outputter(const T& t) : t(t) {}
            const T& t;
        };
    }
    
    template<typename T>
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
        
        template <template <typename ...> class tuple, typename... Ts>
        void output_container(std::ostream& s, const tuple<Ts...>& tup)
        {
            s << "(";
            auto f = [&s](auto... tail) { output_tuple(s, std::forward<Ts>(tail)... ); };
            apply(f, tup);
            s << ")";
        }
        
        template<typename... Ts>
        void output(std::ostream& s, const std::tuple<Ts...>& tup, const std::false_type&)
        {
            output_container(s, tup);
        }

        // Pair support
        template<typename First, typename Second>
        void output(std::ostream& s, const std::pair<First, Second>& tup, const std::false_type&)
        {
            output_container(s, tup);
        }

        template<typename T>
        void output_range_or_type(std::ostream& s, const T& t, const std::false_type&)
        {
            output(s, t, is_streamable<T>{});
        }
        
        template<typename T>
        void output_range_or_type(std::ostream& s, const T& t, const std::true_type&)
        {
            s << "[\n";
            for (auto&& value : t)
                s << "  " << stream_any(value) << ",\n";
            s << "]\n";
        }

        //  We do not want these to go down the container type route just displays as strings
        template <size_t N>
        void output_range_or_type(std::ostream& s, const char(&t)[N], const std::true_type&)
        {
            output(s, t, std::true_type{});
        }

        void output_range_or_type(std::ostream& s, const std::string& t, const std::true_type&)
        {
            output(s, t, std::true_type{});
        }
        
        template<typename T>
        std::ostream& operator<<(std::ostream& s, const outputter<T> & t)
        {	// this call here will dispatch to a function that streams or not depending on whether
            // T has a suitable operator<<.
            output_range_or_type(s, t.t, is_container<T>{});
            return s;
        }
    }
    
    // the manipulator creation function - ie the interface to all of the above
    // to use :
    //     std::cout << stream_any(myobject);
    template<typename T>
    typename stream_any_details::outputter<T> stream_any(const T& t)
    {
        return stream_any_details::outputter<T>{t};
    }
    
    // a useful helper function, a bit like lexical cast, but it will only convert to string, not to other types.
    template<typename T>
    std::string any_to_string(const T& t)
    {
        std::ostringstream os;
        os << stream_any(t);
        return os.str();
    }
}

#endif


