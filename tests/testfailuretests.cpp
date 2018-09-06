#include "testframework/MiniTestFramework.h"
#include "testframework/testfailure.h"

#include <string>

TEST(msg)
{
    auto f = UnitTests::TestFailure("message", "filename", 120);
    ASSERT_EQUALS(std::string("filename(120) : error A1000: Assertion failure : message"), f.what());
}

TEST(msg1)
{
    auto f = UnitTests::TestFailure("different", "filename", 120);
    ASSERT_EQUALS(std::string("filename(120) : error A1000: Assertion failure : different"), f.what());
}

TEST(timeout_msg)
{
    auto f = UnitTests::TestTimeout("message", "filename", 120);
    ASSERT_EQUALS(std::string("filename(120) : error A1001: Test timeout : message"), f.what());
}

TEST(timeout_msg1)
{
    auto f = UnitTests::TestTimeout("different", "filename", 120);
    ASSERT_EQUALS(std::string("filename(120) : error A1001: Test timeout : different"), f.what());
}
