#include <ctime>
#include <fstream>
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
        enum
        {
            Passed,
            Failed,
            Skipped,
            Error,
            Test
        };

        virtual void start_test(std::string suite, std::string test, std::string base) = 0;

        virtual void add_failure(std::string msg) = 0;

        virtual void add_error(std::string msg) = 0;

        virtual void add_skipped() = 0;

        virtual void end_test() = 0;

        virtual void report() = 0;

        virtual ~Reporter() = default;
    };

    class ReporterCommon : public Reporter
    {
    public:
        void start_test(std::string suite, std::string test, std::string base_name) override
        {
            m_current_suite = std::move(suite);
            m_current_test  = std::move(test);
            m_current_base  = std::move(base_name);
            m_error         = Passed;
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

        int get_error() const
        {
            return m_error;
        }

        void end_test() override
        {
            m_results[m_current_suite].emplace_back(m_current_test, m_current_base, get_error(), m_msg);
        }

        struct results
        {
            std::string name;
            std::string base_name;
            int         error;
            std::string msg;

            results(std::string n, std::string b, int e, std::string m)
                : name(std::move(n)), base_name(std::move(b)), error(e), msg(std::move(m))
            {
            }
        };

        int count_tests(const std::vector<results>& tests, int error_type)
        {
            return std::count_if(begin(tests), end(tests),
                [error_type](const results& result) { return error_type == Test || result.error == error_type; });
        }

        int count_tests(int error_type)
        {
            auto num_tests = 0;
            for (auto& suite : m_results)
            {
                auto tests = suite.second;
                num_tests += count_tests(tests, error_type);
            }
            return num_tests;
        }

    protected:
        const std::map<std::string, std::vector<results>>& get_results() const
        {
            return m_results;
        }

    private:
        std::string m_current_suite;
        std::string m_current_test;
        std::string m_current_base;
        std::string m_msg;
        int         m_error = Passed;

        std::map<std::string, std::vector<results>> m_results;
    };

    class StreamReporter : public ReporterCommon
    {
    public:
        using super = ReporterCommon;
        StreamReporter(std::ostream& os, bool verbose) : m_os(os), m_verbose(verbose)
        {
        }

        void start_test(std::string suite, std::string test, std::string base) override
        {
            super::start_test(std::move(suite), std::move(test), std::move(base));
            if (m_verbose)
            {
                print(m_os, "Running ", test, " ");
            }
        }

        void end_test() override
        {
            switch (get_error())
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
            super::end_test();
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
        }

        int show_results(int error_type, const std::string& msg)
        {
            auto num_errors = count_tests(error_type);
            if (num_errors)
            {
                print(m_os, msg, " :-\n");
                for (auto& suite : get_results())
                {
                    auto tests = suite.second;
                    for (auto& result : tests)
                    {
                        if (result.error == error_type)
                        {
                            //  Show err0r
                            print(m_os, "In suite: ", suite.first, " ", result.msg, " while testing TEST(", result.name,
                                ")\n");
                        }
                    }
                }
            }

            return num_errors;
        }

    private:
        std::ostream& m_os;
        bool          m_verbose;
    };

    class XMLReporter : public ReporterCommon
    {
    public:
        using super = ReporterCommon;
        XMLReporter(const std::string& filename) : m_filename(filename)
        {
        }

        void write_testcase(
            std::ostream& s, const results& result, const std::string& classname, const std::string& indent)
        {
            if (result.error == Passed)
            {
                s << indent << "<testcase classname=\"" << classname << "\" name=\"" << result.base_name << "\" />\n";
            }
            else
            {
                s << indent << "<testcase classname=\"" << classname << "\" name=\"" << result.base_name << "\">\n";
                auto new_indent = indent + "  ";
                if (result.error == Skipped)
                {
                    s << new_indent << "<skipped message=\"" << result.msg << "\"/>\n";
                }
                else if (result.error == Failed)
                {
                    s << new_indent << "<Failure message=\"Test Failure\">\n";
                    s << result.msg << "\n";
                    s << new_indent << "</Failure>\n";
                }
                else if (result.error == Error)
                {
                    s << new_indent << "<Error message=\"Unexpected Exception encountered\">\n";
                    s << result.msg << "\n";
                    s << new_indent << "</Error>\n";
                }
                s << indent << "</testcase>\n";
            }
        }

        void write_suite(std::ostream& s, const std::string& name, const std::vector<results>& results,
            const std::string& indent, int id)
        {
            auto tests    = count_tests(results, Test);
            auto failures = count_tests(results, Failed);
            auto errors   = count_tests(results, Error);
            auto skipped  = count_tests(results, Skipped);

            s << indent << "<testsuite id=\"" << id << "\" name=\"" << name << "\" tests=\"" << tests
              << "\" failures=\"" << failures << "\" errors=\"" << errors << "\" skipped=\"" << skipped << "\">\n";

            for (auto& result : results)
            {
                write_testcase(s, result, name, indent + "  ");
            }

            s << indent << "</testsuite>\n";
        }

        void report() override
        {
            auto f = std::ofstream(m_filename);
            f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            f << "<testsuites>\n";

            int id = 0;
            for (auto& result : get_results())
            {
                write_suite(f, result.first, result.second, "  ", id++);
            }
            f << "</testsuites>\n";
        }

    private:
        std::string m_filename;
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
    std::string MiniSuite::Test::Suite() const
    {
        return m_suite;
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

    std::string FindXMLFilename(const std::vector<std::string>& args)
    {
        auto pos = std::find_if(begin(args), end(args), [](const std::string& s) { return s == "--xml" || s == "-x"; });
        if (pos != end(args))
        {
            pos = std::next(pos);
            if (pos != end(args))
                return *pos;
            else
            {
                throw std::runtime_error("You must provide a filename for the xml report.");
            }
        }

        return "";
    }

    int MiniSuite::RunTests(const std::vector<std::string>& args, std::ostream& os)
    {
        auto verbose    = IsVerbose(args);
        auto start_time = clock();

        auto xml      = FindXMLFilename(args);
        auto reporter = xml.empty() ? std::unique_ptr<Reporter>(std::make_unique<StreamReporter>(os, verbose)) :
                                      std::unique_ptr<Reporter>(std::make_unique<XMLReporter>(xml));

        auto num_tests = run_tests(tests, verbose, *reporter);
        auto end_time  = clock();
        reporter->report();
        print(os, "\nTime taken = ", 1000.0 * (end_time - start_time) / CLOCKS_PER_SEC, "ms\n");
        return static_cast<int>(failures.size());
    }

    int MiniSuite::run_tests(std::vector<std::unique_ptr<Test>>& tests, bool verbose, Reporter& reporter)
    {
        auto num_tests = 0U;
        for (auto& test : tests)
        {
            for (auto index = 0; index != test->NumTests(); ++index)
            {
                auto indexs = std::string(test->NumTests() == 1 ? "" : "[" + std::to_string(index) + "]");
                reporter.start_test(test->Suite(), test->Name(indexs), test->BareName(indexs));
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
    try
    {
        auto end_argv = std::next(argv, argc);
        auto args     = std::vector<std::string>(argv, end_argv);
        return UnitTests::MiniSuite::Instance().RunTests(args, std::cout);
    }
    catch (std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return -1;
    }
} /**/
