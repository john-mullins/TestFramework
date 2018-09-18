#include <ctime>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace UnitTests
{
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

    class Reporter
    {
    public:
        virtual void start_test(std::string suite, std::string test) = 0;

        virtual void add_failure(std::string msg) = 0;

        virtual void add_error(std::string msg) = 0;

        virtual void add_skipped() = 0;

        virtual void end_test() = 0;

        virtual void report() = 0;

        virtual ~Reporter() = default;
    };

    class StreamReporter : public Reporter
    {
    public:
        enum
        {
            Passed,
            Failed,
            Skipped,
            Error,
            Test
        };

        StreamReporter(std::ostream& os, bool verbose) : m_os(os), m_verbose(verbose), m_error(Passed)
        {
        }

        void start_test(std::string suite, std::string test) override
        {
            m_current_suite = suite;
            m_current_test  = test;
            m_error         = Passed;
            if (m_verbose)
            {
                print(m_os, "Running ", m_current_test, " ");
            }
        }

        void add_failure(std::string msg) override
        {
            m_error = Failed;
            m_msg   = std::move(msg);
        }

        void add_error(std::string msg) override
        {
            m_error = Error;
            m_msg   = std::move(msg);
        }

        void add_skipped() override
        {
            m_error = Skipped;
        }

        void end_test() override
        {
            switch (m_error)
            {
                case Passed:
                    print(m_os, m_verbose ? "OK\n" : ".");
                    break;
                case Failed:
                    print(m_os, m_verbose ? "FAIL\n" : "F");
                    break;
                case Skipped:
                    print(m_os, m_verbose ? "SKIP\n" : "S");
                    break;
                case Error:
                    print(m_os, m_verbose ? "ERROR\n" : "E");
                    break;
            }
            m_results[m_current_suite].emplace_back(m_current_test, m_error, m_msg);
        }

        void report() override
        {
            print(m_os, "\n");
            auto errors   = show_results(Error, "Errors");
            auto failures = show_results(Failed, "Failures");
            auto skipped  = show_results(Skipped, "Skipped");

            print(m_os, count_tests(Test), " Tests.\n");
            print(m_os, skipped, " Skipped.\n");
            print(m_os, failures, " Failures.\n");
            print(m_os, errors, " Errors.\n");

            // show_stats();
        }

        int count_tests(int error_type)
        {
            auto num_tests = 0;
            for (auto& suite : m_results)
            {
                auto tests = suite.second;
                num_tests += std::count_if(begin(tests), end(tests),
                    [error_type](const results& result) { return error_type == Test || result.error == error_type; });
            }
            return num_tests;
        }

        int show_results(int error_type, const std::string& msg)
        {
            auto num_errors = count_tests(error_type);
            if (num_errors)
            {
                print(m_os, msg, " :-\n");
                for (auto& suite : m_results)
                {
                    auto tests = suite.second;
                    for (auto& result : tests)
                    {
                        if (result.error == error_type)
                        {
                            //  Show err0r
                            print(m_os, result.msg, " while testing TEST(", result.name, ")\n");
                        }
                    }
                }
            }

            return num_errors;
        }

    private:
        std::ostream& m_os;
        bool          m_verbose;
        std::string   m_current_suite;
        std::string   m_current_test;
        int           m_error;
        std::string   m_msg;

        struct results
        {
            std::string name;
            int         error;
            std::string msg;

            results(std::string n, int e, std::string m) : name(std::move(n)), error(e), msg(std::move(m))
            {
            }
        };

        std::map<std::string, std::vector<results>> m_results;
    };

    std::string MiniSuite::Test::BareName(const std::string& indexs) const
    {
        return m_name + indexs;
    }

    std::string MiniSuite::Test::Name(const std::string& indexs) const
    {
        std::stringstream s;
        s << BareName(indexs) << " @ " << m_file << ':' << m_line;
        return s.str();
    }

    size_t MiniSuite::AddTest(std::unique_ptr<Test> test)
    {
        tests.push_back(std::move(test));
        return 0;
    }

    bool MiniSuite::IsVerbose(const std::vector<std::string>& args)
    {
        return std::any_of(begin(args), end(args), [](const auto& s) { return s == "-v" || s == "--verbose"; });
    }

    void MiniSuite::show_failures(
        std::ostream& os, const std::vector<std::pair<std::string, std::string>> failures, const std::string& type)
    {
        if (!failures.empty())
        {
            print(os, type, " :-\n");
            std::for_each(begin(failures), end(failures),
                [&](auto f) { print(os, f.second, " while testing TEST(", f.first, ")\n"); });
        }
    }

    int MiniSuite::RunTests(const std::vector<std::string>& args, std::ostream& os)
    {
        auto                      verbose    = IsVerbose(args);
        auto                      start_time = clock();
        std::unique_ptr<Reporter> reporter{new StreamReporter(os, verbose)};

        auto num_tests = run_tests(tests, verbose, *reporter);
        reporter->report();
        auto end_time = clock();
        print(os, "\nTime taken = ", 1000.0 * (end_time - start_time) / CLOCKS_PER_SEC, "ms\n");
        return static_cast<int>(failures.size());
    }

    int MiniSuite::run_tests(std::vector<std::unique_ptr<Test>>& tests, bool verbose, Reporter& reporter)
    {
        auto num_tests = 0U;
        for (auto& test : tests)
        {
            for (int index = 0; index != test->NumTests(); ++index)
            {
                auto indexs = std::string(test->NumTests() == 1 ? "" : "[" + std::to_string(index) + "]");
                reporter.start_test("anon", test->Name(indexs));
                try
                {
                    test->Run(index);
                }
                catch (TestFailure& e)
                {
                    reporter.add_failure(e.what());
                }
                catch (const std::exception& e)
                {
                    reporter.add_error(e.what());
                }
                reporter.end_test();
            }
        }
        return num_tests;
    }

    MiniSuite& MiniSuite::Instance()
    {
        static UnitTests::MiniSuite runner;
        return runner;
    }

    std::string FormatError(std::string file, int line, int error)
    {
        auto msg(std::move(file));
        msg += "(" + std::to_string(line) + ")";
        msg += " : error A" + std::to_string(error) + ": ";
        return msg;
    }
} // namespace UnitTests

int main(int argc, char** argv)
{
    auto end_argv = std::next(argv, argc);
    auto args     = std::vector<std::string>(argv, end_argv);
    return UnitTests::MiniSuite::Instance().RunTests(args, std::cout);
} /**/
