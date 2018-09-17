#if !defined(IMG_MiniTestFramework_h_)
#define IMG_MiniTestFramework_h_

#include "TestHelpers.h"
#include "assertions.h"
#include "testfailure.h"

#include <memory>
#include <sstream>
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

    class MiniSuite
    {
    public:
        static MiniSuite& Instance();

        class Test
        {
        public:
            Test(std::string name, const char* file, int line) : m_name(std::move(name)), m_file(file), m_line(line)
            {
            }

            virtual void Run(int) const   = 0;
            virtual int  NumTests() const = 0;
            virtual ~Test()               = default;

            std::string BareName(const std::string& indexs) const;

            std::string Name(const std::string& indexs) const;

        private:
            std::string m_name;
            const char* m_file;
            int         m_line;
        };

        template <class Function>
        class FunctionTest : public Test
        {
        public:
            FunctionTest(Function fn, std::string name, const char* file, int line)
                : Test(std::move(name), file, line), m_fn(fn)
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

            ParamFunctionTest(Container cont, const Function& fn, std::string name, const char* file, int line)
                : Test(std::move(name), file, line), m_cont(std::move(cont)), m_fn(fn)
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
        size_t AddTest(Function fn, const char* name, const char* file, int line)
        {
            auto test = std::unique_ptr<Test>(std::make_unique<FunctionTest<Function>>(fn, name, file, line));
            return AddTest(std::move(test));
        }

        template <class Container, class Function>
        size_t AddParamTest(const Container& cont, Function fn, const char* name, const char* file, int line)
        {
            auto test = std::unique_ptr<Test>(
                std::make_unique<ParamFunctionTest<Container, Function>>(cont, fn, name, file, line));
            return AddTest(std::move(test));
        }

        template <typename T, size_t N, class Function>
        size_t AddParamTest(const T (&data)[N], Function fn, const char* name, const char* file, int line)
        {
            using std::begin;
            using std::end;
            return AddParamTest(std::vector<T>(begin(data), end(data)), fn, name, file, line);
        }

        bool IsVerbose(const std::vector<std::string>& args);

        void show_failures(
            std::ostream& os, const std::vector<std::pair<std::string, std::string>> failures, const std::string& type);

        int RunTests(const std::vector<std::string>& args, std::ostream& os);

    private:
        std::vector<std::unique_ptr<Test>>               tests;
        std::vector<std::pair<std::string, std::string>> failures;
        std::vector<std::pair<std::string, std::string>> errors;

        int run_tests(std::vector<std::unique_ptr<Test>>& tests, bool verbose, std::ostream& os);
    };

#define TEST(name)                                                                         \
    void name();                                                                           \
    namespace                                                                              \
    {                                                                                      \
        namespace PP_CAT(unique, __LINE__)                                                 \
        {                                                                                  \
            const size_t ignore_this_warning =                                             \
                UnitTests::MiniSuite::Instance().AddTest(name, #name, __FILE__, __LINE__); \
        }                                                                                  \
    }                                                                                      \
    void name() /**/

#define TEST_T(name, list_of_types)                                                                        \
    template <typename test_type>                                                                          \
    struct name                                                                                            \
    {                                                                                                      \
        void operator()() const;                                                                           \
    };                                                                                                     \
    namespace                                                                                              \
    {                                                                                                      \
        namespace PP_CAT(unique, __LINE__)                                                                 \
        {                                                                                                  \
            template <typename... T>                                                                       \
            struct expander;                                                                               \
                                                                                                           \
            template <typename T>                                                                          \
            struct expander<T>                                                                             \
            {                                                                                              \
                int operator()() const                                                                     \
                {                                                                                          \
                    return UnitTests::MiniSuite::Instance().AddTest(name<T>(), #name, __FILE__, __LINE__); \
                }                                                                                          \
            };                                                                                             \
                                                                                                           \
            template <typename T, typename... Args>                                                        \
            struct expander<T, Args...>                                                                    \
            {                                                                                              \
                int operator()() const                                                                     \
                {                                                                                          \
                    return expander<T>()(), expander<Args...>()();                                         \
                }                                                                                          \
            };                                                                                             \
                                                                                                           \
            template <typename... Args>                                                                    \
            struct expander<UnitTests::typelist<Args...>>                                                  \
            {                                                                                              \
                int operator()() const                                                                     \
                {                                                                                          \
                    return expander<Args...>()();                                                          \
                }                                                                                          \
            };                                                                                             \
            const size_t ignore_this_warning = expander<list_of_types>()();                                \
        }                                                                                                  \
    }                                                                                                      \
    template <typename test_type>                                                                          \
    void name<test_type>::operator()() const /**/

#define PARAM_TEST(name, data)                                                                          \
    struct name                                                                                         \
    {                                                                                                   \
        template <typename T>                                                                           \
        void operator()(const T& args) const;                                                           \
    };                                                                                                  \
    namespace                                                                                           \
    {                                                                                                   \
        namespace PP_CAT(unique, __LINE__)                                                              \
        {                                                                                               \
            const size_t ignore_this_warning =                                                          \
                UnitTests::MiniSuite::Instance().AddParamTest(data, name(), #name, __FILE__, __LINE__); \
        }                                                                                               \
    }                                                                                                   \
    template <typename T>                                                                               \
    void name::operator()(const T& args) const /**/

#define PARAM_TEST_T(name, data, list_of_types)                                                                       \
    template <typename U>                                                                                             \
    struct name                                                                                                       \
    {                                                                                                                 \
        template <typename T>                                                                                         \
        void operator()(const T& args) const;                                                                         \
    };                                                                                                                \
    namespace                                                                                                         \
    {                                                                                                                 \
        namespace PP_CAT(unique, __LINE__)                                                                            \
        {                                                                                                             \
            template <typename... T>                                                                                  \
            struct expander;                                                                                          \
                                                                                                                      \
            template <typename T>                                                                                     \
            struct expander<T>                                                                                        \
            {                                                                                                         \
                int operator()() const                                                                                \
                {                                                                                                     \
                    return UnitTests::MiniSuite::Instance().AddParamTest(data, name<T>(), #name, __FILE__, __LINE__); \
                }                                                                                                     \
            };                                                                                                        \
                                                                                                                      \
            template <typename T, typename... Args>                                                                   \
            struct expander<T, Args...>                                                                               \
            {                                                                                                         \
                int operator()() const                                                                                \
                {                                                                                                     \
                    return expander<T>()(), expander<Args...>()();                                                    \
                }                                                                                                     \
            };                                                                                                        \
            template <typename... Args>                                                                               \
            struct expander<UnitTests::typelist<Args...>>                                                             \
            {                                                                                                         \
                int operator()() const                                                                                \
                {                                                                                                     \
                    return expander<Args...>()();                                                                     \
                }                                                                                                     \
            };                                                                                                        \
            const size_t ignore_this_warning = expander<list_of_types>()();                                           \
        }                                                                                                             \
    }                                                                                                                 \
    template <typename test_type>                                                                                     \
    template <typename T>                                                                                             \
    void name<test_type>::operator()(const T& args) const /**/

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

#ifdef TEST_MAIN
#include "testmain.inl"
#endif

#endif
