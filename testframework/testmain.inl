#include <ctime>
#include <iostream>

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
        auto verbose    = IsVerbose(args);
        auto start_time = clock();
        auto num_tests  = run_tests(tests, verbose, os);
        print(os, "\n");
        auto end_time = clock();
        show_failures(os, errors, "Errors");
        show_failures(os, failures, "Failures");
        print(os, num_tests, " Tests.\n");
        print(os, 0, " Skipped.\n");
        print(os, failures.size(), " Failures.\n");
        print(os, errors.size(), " Errors.\n");
        print(os, "\nTime taken = ", 1000.0 * (end_time - start_time) / CLOCKS_PER_SEC, "ms\n");
        return static_cast<int>(failures.size());
    }

    int MiniSuite::run_tests(std::vector<std::unique_ptr<Test>>& tests, bool verbose, std::ostream& os)
    {
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
        return num_tests;
    }
} // namespace UnitTests

UnitTests::MiniSuite& UnitTests::MiniSuite::Instance()
{
    static UnitTests::MiniSuite runner;
    return runner;
}

std::string UnitTests::FormatError(std::string file, int line, int error)
{
    auto msg(std::move(file));
    msg += "(" + std::to_string(line) + ")";
    msg += " : error A" + std::to_string(error) + ": ";
    return msg;
}

int main(int argc, char** argv)
{
    auto end_argv = std::next(argv, argc);
    auto args     = std::vector<std::string>(argv, end_argv);
    return UnitTests::MiniSuite::Instance().RunTests(args, std::cout);
} /**/
