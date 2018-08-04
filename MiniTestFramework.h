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

            std::string m_name;
            const char* m_file;
            int         m_line;
        };

        class FunctionTest : public Test
        {
        public:
            FunctionTest(void (*fn)(), const std::string& name, const char* file, int line)
                : Test(name, file, line), m_fn(fn)
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
            void (*m_fn)();
        };

        template <class Container, class Function>
        class ParamFunctionTest : public Test
        {
        public:
            using param_type = typename Container::value_type;

            ParamFunctionTest(Container cont, const Function& fn, const std::string& name, const char* file, int line)
                : Test(name, file, line), m_cont(std::move(cont)), m_fn(fn)
            {
            }

            int NumTests() const override
            {
                return static_cast<int>(std::distance(m_cont.begin(), m_cont.end()));
            }

            void Run(int index) const override
            {
                auto arg = m_cont.begin();
                std::advance(arg, index);
                m_fn(*arg);
            }

        private:
            Container m_cont;
            Function  m_fn;
        };

        size_t AddTest(std::unique_ptr<Test> test)
        {
            tests.push_back(std::move(test));
            return 0;
        }

        size_t AddTest(void (*fn)(), const std::string& name, const char* file, int line)
        {
            auto test = std::unique_ptr<Test>(std::make_unique<FunctionTest>(fn, name, file, line));
            return AddTest(std::move(test));
        }

        template <class Container, class Function>
        size_t AddParamTest(const Container& cont, const Function& fn, const char* name, const char* file, int line)
        {
            // auto test = std::make_unique<ParamFunctionTest<Container, Function>>(cont, fn, name, file, line);
            auto test = std::unique_ptr<Test>(
                std::make_unique<ParamFunctionTest<Container, Function>>(cont, fn, name, file, line));
            return AddTest(std::move(test));
        }

        template <typename T, size_t N, class Function>
        size_t AddParamTest(const T (&data)[N], const Function& fn, const char* name, const char* file, int line)
        {
            using std::begin, std::end;
            return AddParamTest(std::vector<T>(begin(data), end(data)), fn, name, file, line);
        }

        bool IsVerbose(const std::vector<std::string>& args)
        {
            return std::any_of(begin(args), end(args), [](auto s) { return s == "-v" || s == "--verbose"; });
        }

#define PRINTF printf

        int RunTests(const std::vector<std::string>& args)
        {
            auto verbose   = IsVerbose(args);
            auto failures  = std::vector<std::pair<std::string, std::string>>{};
            auto errors    = 0U;
            auto num_tests = 0U;
            for (auto& test : tests)
            {
                for (int index = 0; index != test->NumTests(); ++index)
                {
                    auto indexs = std::string(test->NumTests() == 1 ? "" : "[" + std::to_string(index) + "]");
                    auto name   = test->m_name + indexs;
                    if (verbose)
                    {
                        PRINTF("Running %s ", name.c_str());
                    }
                    try
                    {
                        ++num_tests;
                        test->Run(index);
                        PRINTF(!verbose ? "." : "OK\n");
                    }
                    catch (TestFailure& e)
                    {
                        failures.emplace_back(name, e.what());
                        PRINTF(!verbose ? "F" : "FAIL\n");
                    }
                    catch (const std::exception& e)
                    {
                        failures.emplace_back(name, e.what());
                        PRINTF(!verbose ? "E" : "EXCEPTION\n");
                        ++errors;
                    }
                }
            }
            PRINTF("\n");
            if (!failures.empty())
            {
                PRINTF("Failures :-\n");
                for (auto&& failure : failures)
                {
                    PRINTF("%s  while testing TEST(%s)\n", failure.second.c_str(), failure.first.c_str());
                }
            }
            PRINTF("%d Tests.\n", static_cast<int>(num_tests));
            PRINTF("%d Skipped.\n", 0);
            PRINTF("%d Failures.\n", static_cast<int>(failures.size() - errors));
            PRINTF("%d Errors.\n", static_cast<int>(errors));
            return static_cast<int>(failures.size());
        }

    private:
        std::vector<std::unique_ptr<Test>> tests;
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

#define TEST_MAIN()                                                                  \
    UnitTests::MiniSuite& UnitTests::MiniSuite::Instance()                           \
    {                                                                                \
        static UnitTests::MiniSuite runner;                                          \
        return runner;                                                               \
    }                                                                                \
    std::string UnitTests::FormatError(const std::string& file, int line, int error) \
    {                                                                                \
        auto msg = file;                                                             \
        msg += "(" + std::to_string(line) + ")";                                     \
        msg += " : error A" + std::to_string(error) + ": ";                          \
        return msg;                                                                  \
    }                                                                                \
    int main(int argc, char** argv)                                                  \
    {                                                                                \
        auto end_argv = argv;                                                        \
        std::advance(end_argv, argc);                                                \
        auto args = std::vector<std::string>(argv, end_argv);                        \
        return UnitTests::MiniSuite::Instance().RunTests(args);                      \
    }                                                                                \
    /**/

} // namespace UnitTests

#endif
