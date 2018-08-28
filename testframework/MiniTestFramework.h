#if !defined(IMG_MiniTestFramework_h_)
#define IMG_MiniTestFramework_h_

#include "TestHelpers.h"
#include "assertions.h"
#include "testfailure.h"
#include <iostream>
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

    template <typename T>
    std::ostream& print(std::ostream& s, const T& arg)
    {
        return s << arg;
    }

    template <typename T, typename... Args>
    std::ostream& print(std::ostream& s, const T& arg, Args... args)
    {
        print(s, arg);
        return print(s, args...);
    }

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

            std::string BareName(const std::string& indexs) const
            {
                return m_name + indexs;
            }

            std::string Name(const std::string& indexs) const
            {
                std::stringstream s;
                s << BareName(indexs) << " @ " << m_file << ':' << m_line;
                return s.str();
            }

        private:
            std::string m_name;
            const char* m_file;
            int         m_line;
        };

        class FunctionTest : public Test
        {
        public:
            FunctionTest(void (*fn)(), std::string name, const char* file, int line)
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
            void (*m_fn)();
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

        size_t AddTest(void (*fn)(), const char* name, const char* file, int line)
        {
            auto test = std::unique_ptr<Test>(std::make_unique<FunctionTest>(fn, name, file, line));
            return AddTest(std::move(test));
        }

        template <class Container, class Function>
        size_t AddParamTest(const Container& cont, const Function& fn, const char* name, const char* file, int line)
        {
            auto test = std::unique_ptr<Test>(
                std::make_unique<ParamFunctionTest<Container, Function>>(cont, fn, name, file, line));
            return AddTest(std::move(test));
        }

        template <typename T, size_t N, class Function>
        size_t AddParamTest(const T (&data)[N], const Function& fn, const char* name, const char* file, int line)
        {
            using std::begin;
            using std::end;
            return AddParamTest(std::vector<T>(begin(data), end(data)), fn, name, file, line);
        }

        bool IsVerbose(const std::vector<std::string>& args)
        {
            return std::any_of(begin(args), end(args), [](const auto& s) { return s == "-v" || s == "--verbose"; });
        }

        void show_failures(
            std::ostream& os, const std::vector<std::pair<std::string, std::string>> failures, const std::string& type)
        {
            if (!failures.empty())
            {
                print(os, type, " :-\n");
                std::for_each(begin(failures), end(failures),
                    [&](auto f) { print(os, f.second, " while testing TEST(", f.first, ")\n"); });
            }
        }

        int RunTests(const std::vector<std::string>& args, std::ostream& os)
        {
            auto verbose   = IsVerbose(args);
            auto failures  = std::vector<std::pair<std::string, std::string>>{};
            auto errors    = std::vector<std::pair<std::string, std::string>>{};
            auto num_tests = 0U;
            for (auto& test : tests)
            {
                for (int index = 0; index != test->NumTests(); ++index)
                {
                    auto indexs = std::string(test->NumTests() == 1 ? "" : "[" + std::to_string(index) + "]");
                    if (verbose)
                    {
                        print(os, "Running ", test->BareName(indexs), " ");
                    }
                    try
                    {
                        ++num_tests;
                        test->Run(index);
                        print(os, !verbose ? "." : "OK\n");
                    }
                    catch (TestFailure& e)
                    {
                        failures.emplace_back(test->Name(indexs), e.what());
                        print(os, !verbose ? "F" : "FAIL\n");
                    }
                    catch (const std::exception& e)
                    {
                        errors.emplace_back(test->Name(indexs), e.what());
                        print(os, !verbose ? "E" : "EXCEPTION\n");
                    }
                }
            }
            print(os, "\n");
            show_failures(os, errors, "Errors");
            show_failures(os, failures, "Failures");
            print(os, num_tests, " Tests.\n");
            print(os, 0, " Skipped.\n");
            print(os, failures.size(), " Failures.\n");
            print(os, errors.size(), " Errors.\n");
            return failures.size();
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

#define TEST_MAIN()                                                           \
    UnitTests::MiniSuite& UnitTests::MiniSuite::Instance()                    \
    {                                                                         \
        static UnitTests::MiniSuite runner;                                   \
        return runner;                                                        \
    }                                                                         \
                                                                              \
    std::string UnitTests::FormatError(std::string file, int line, int error) \
    {                                                                         \
        auto msg(std::move(file));                                            \
        msg += "(" + std::to_string(line) + ")";                              \
        msg += " : error A" + std::to_string(error) + ": ";                   \
        return msg;                                                           \
    }                                                                         \
                                                                              \
    int main(int argc, char** argv)                                           \
    {                                                                         \
        auto end_argv = argv;                                                 \
        std::advance(end_argv, argc);                                         \
        auto args = std::vector<std::string>(argv, end_argv);                 \
        return UnitTests::MiniSuite::Instance().RunTests(args, std::cout);    \
    }                                                                         \
    /**/

} // namespace UnitTests

#endif
