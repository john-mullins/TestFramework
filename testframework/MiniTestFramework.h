#if !defined(IMG_MiniTestFramework_h_)
#define IMG_MiniTestFramework_h_

#include "TestHelpers.h"
#include "assertions.h"
#include "testfailure.h"

#include <memory>
#include <utility>
#include <vector>

namespace UnitTests
{
#define PP_CAT(a, b) PP_CAT_AGAIN(a, b)
#if !defined(_MSC_VER)
#define PP_CAT_AGAIN(a, b) a##b
#else
#define PP_CAT_AGAIN(a, b) PP_CAT_AGAIN_II(~, a##b)
#define PP_CAT_AGAIN_II(p, res) res
#endif

    template <typename... Types>
    struct typelist
    {
    };

    class Reporter;

    class MiniSuite
    {
    public:
        static MiniSuite& Instance();

        class Test
        {
        public:
            Test(std::string suite, std::string name, const char* file, int line)
                : m_suite(std::move(suite)), m_name(std::move(name)), m_file(file), m_line(line)
            {
            }

            virtual void Run(int) const   = 0;
            virtual int  NumTests() const = 0;
            virtual ~Test()               = default;

            std::string BareName(const std::string& indexs) const;

            std::string Name(const std::string& indexs) const;

            std::string Suite() const;

        private:
            std::string m_suite;
            std::string m_name;
            const char* m_file;
            int         m_line;
        };

        template <class Function>
        class FunctionTest : public Test
        {
        public:
            FunctionTest(Function fn, std::string suite, std::string name, const char* file, int line)
                : Test(std::move(suite), std::move(name), file, line), m_fn(fn)
            {
            }

            void Run(int /*unused*/) const override
            {
                m_fn();
            }

            int NumTests() const override
            {
                return 1;
            }

        private:
            Function m_fn;
        };

        template <class Container, class Function>
        class ParamFunctionTest : public Test
        {
        public:
            using param_type = typename Container::value_type;

            ParamFunctionTest(
                Container cont, const Function& fn, std::string suite, std::string name, const char* file, int line)
                : Test(std::move(suite), std::move(name), file, line), m_cont(std::move(cont)), m_fn(fn)
            {
            }

            int NumTests() const override
            {
                return static_cast<int>(std::distance(m_cont.begin(), m_cont.end()));
            }

            void Run(int index) const override
            {
                auto arg = std::next(m_cont.begin(), index);
                m_fn(*arg);
            }

        private:
            Container m_cont;
            Function  m_fn;
        };

        size_t AddTest(std::unique_ptr<Test> test);

        template <class Function>
        size_t AddTest(Function fn, const char* suite, const char* name, const char* file, int line)
        {
            auto test = std::unique_ptr<Test>(new FunctionTest<Function>(fn, suite, name, file, line));
            return AddTest(std::move(test));
        }

        template <class Container, class Function>
        size_t AddParamTest(
            const Container& cont, Function fn, const char* suite, const char* name, const char* file, int line)
        {
            auto test =
                std::unique_ptr<Test>(new ParamFunctionTest<Container, Function>(cont, fn, suite, name, file, line));
            return AddTest(std::move(test));
        }

        template <typename T, size_t N, class Function>
        size_t AddParamTest(
            const T (&data)[N], Function fn, const char* suite, const char* name, const char* file, int line)
        {
            using std::begin;
            using std::end;
            return AddParamTest(std::vector<T>(begin(data), end(data)), fn, suite, name, file, line);
        }

        bool IsVerbose(const std::vector<std::string>& args);

        void show_failures(
            std::ostream& os, const std::vector<std::pair<std::string, std::string>> failures, const std::string& type);

        int RunTests(const std::vector<std::string>& args, std::ostream& os);

    private:
        std::vector<std::unique_ptr<Test>> tests;

        int run_tests(std::vector<std::unique_ptr<Test>>& tests, Reporter& reporter);
    };

#define _TEST1(name) _TEST(test_suite, name)

#define _TEST2(suite, name) _TEST(#suite, name)

#define _TEST(suite, name)                                                                        \
    void name();                                                                                  \
    namespace                                                                                     \
    {                                                                                             \
        namespace PP_CAT(unique, __LINE__)                                                        \
        {                                                                                         \
            const size_t ignore_this_warning =                                                    \
                UnitTests::MiniSuite::Instance().AddTest(name, suite, #name, __FILE__, __LINE__); \
        }                                                                                         \
    }                                                                                             \
    void name() /**/

#define TEST_T(suite, name, list_of_types)                                                                         \
    template <typename test_type>                                                                                  \
    struct name                                                                                                    \
    {                                                                                                              \
        void operator()() const;                                                                                   \
    };                                                                                                             \
    namespace                                                                                                      \
    {                                                                                                              \
        namespace PP_CAT(unique, __LINE__)                                                                         \
        {                                                                                                          \
            template <typename... T>                                                                               \
            struct expander;                                                                                       \
                                                                                                                   \
            template <typename T>                                                                                  \
            struct expander<T>                                                                                     \
            {                                                                                                      \
                size_t operator()() const                                                                          \
                {                                                                                                  \
                    return UnitTests::MiniSuite::Instance().AddTest(name<T>(), #suite, #name, __FILE__, __LINE__); \
                }                                                                                                  \
            };                                                                                                     \
                                                                                                                   \
            template <typename T, typename... Args>                                                                \
            struct expander<T, Args...>                                                                            \
            {                                                                                                      \
                size_t operator()() const                                                                          \
                {                                                                                                  \
                    return expander<T>()(), expander<Args...>()();                                                 \
                }                                                                                                  \
            };                                                                                                     \
                                                                                                                   \
            template <typename... Args>                                                                            \
            struct expander<UnitTests::typelist<Args...>>                                                          \
            {                                                                                                      \
                size_t operator()() const                                                                          \
                {                                                                                                  \
                    return expander<Args...>()();                                                                  \
                }                                                                                                  \
            };                                                                                                     \
            const size_t ignore_this_warning = expander<list_of_types>()();                                        \
        }                                                                                                          \
    }                                                                                                              \
    template <typename test_type>                                                                                  \
    void name<test_type>::operator()() const /**/

#define _PARAM_TEST2(name, data) _PARAM_TEST(test_suite, name, data)

#define _PARAM_TEST3(suite, name, data) _PARAM_TEST(#suite, name, data)

#define _PARAM_TEST(suite, name, data)                                                                         \
    struct name                                                                                                \
    {                                                                                                          \
        template <typename T>                                                                                  \
        void operator()(const T& args) const;                                                                  \
    };                                                                                                         \
    namespace                                                                                                  \
    {                                                                                                          \
        namespace PP_CAT(unique, __LINE__)                                                                     \
        {                                                                                                      \
            const size_t ignore_this_warning =                                                                 \
                UnitTests::MiniSuite::Instance().AddParamTest(data, name(), suite, #name, __FILE__, __LINE__); \
        }                                                                                                      \
    }                                                                                                          \
    template <typename T>                                                                                      \
    void name::operator()(const T& args) const /**/

#define PARAM_TEST_T(suite, name, data, list_of_types)                       \
    template <typename U>                                                    \
    struct name                                                              \
    {                                                                        \
        template <typename T>                                                \
        void operator()(const T& args) const;                                \
    };                                                                       \
    namespace                                                                \
    {                                                                        \
        namespace PP_CAT(unique, __LINE__)                                   \
        {                                                                    \
            template <typename... T>                                         \
            struct expander;                                                 \
                                                                             \
            template <typename T>                                            \
            struct expander<T>                                               \
            {                                                                \
                size_t operator()() const                                    \
                {                                                            \
                    return UnitTests::MiniSuite::Instance().AddParamTest(    \
                        data, name<T>(), #suite, #name, __FILE__, __LINE__); \
                }                                                            \
            };                                                               \
                                                                             \
            template <typename T, typename... Args>                          \
            struct expander<T, Args...>                                      \
            {                                                                \
                size_t operator()() const                                    \
                {                                                            \
                    return expander<T>()(), expander<Args...>()();           \
                }                                                            \
            };                                                               \
            template <typename... Args>                                      \
            struct expander<UnitTests::typelist<Args...>>                    \
            {                                                                \
                size_t operator()() const                                    \
                {                                                            \
                    return expander<Args...>()();                            \
                }                                                            \
            };                                                               \
            const size_t ignore_this_warning = expander<list_of_types>()();  \
        }                                                                    \
    }                                                                        \
    template <typename test_type>                                            \
    template <typename T>                                                    \
    void name<test_type>::operator()(const T& args) const /**/

#define EXPAND(x) x
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define TEST(...) EXPAND(GET_MACRO(__VA_ARGS__, _TEST3, _TEST2, _TEST1, _UNUSED)(__VA_ARGS__))
#define PARAM_TEST(...) EXPAND(GET_MACRO(__VA_ARGS__, _PARAM_TEST3, _PARAM_TEST2, _UNUSED)(__VA_ARGS__))

#define ADD_TESTS(name, data) UnitTests::MiniSuite::Instance().AddParamTest(data, name, #name, __FILE__, __LINE__);

#define TEST_INITIALIZER(name)                               \
    struct name                                              \
    {                                                        \
        template <typename T>                                \
        void operator()(T) const;                            \
    };                                                       \
    namespace                                                \
    {                                                        \
        namespace PP_CAT(unique, __LINE__)                   \
        {                                                    \
            inline size_t initialize()                       \
            {                                                \
                name()(1);                                   \
                return 0;                                    \
            }                                                \
            const size_t ignore_this_warning = initialize(); \
        }                                                    \
    }                                                        \
    template <typename T>                                    \
    void name::operator()(T) const

} // namespace UnitTests

static const char* test_suite = "anonymous";

#ifdef TEST_MAIN
#include "testmain.inl"
#endif

#endif
