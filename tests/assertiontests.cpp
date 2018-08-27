#include "testframework/MiniTestFramework.h"

using namespace std::literals;

TEST(fail)
{
    try
    {
        FAIL();
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Test FAIL'ed";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Fail failed", __FILE__, __LINE__);
        }
    }
}

TEST(fail_msg)
{
    try
    {
        FAIL("More Info");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : More Info";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Fail failed", __FILE__, __LINE__);
        }
    }
}

TEST(equals)
{
    try
    {
        ASSERT_EQUALS(1, 1);
    }
    catch (UnitTests::TestFailure& e)
    {
        FAIL();
    }
}

TEST(equals1)
{
    try
    {
        ASSERT_EQUALS("Hello", "World");
        FAIL();
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg = e.what();
        std::string expected =
            "error A1000: Assertion failure : \n    Expected <Hello (uncoerced=Hello)>\n     but got "
            "<World (uncoerced=World)>";
        // std::cout << "\n" << msg << "\n" << expected << "\n";

        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Fail failed", __FILE__, __LINE__);
        }
    }
}

TEST(equals_msg)
{
    try
    {
        ASSERT_EQUALS("Message", "Hello"s, "World"s);
        FAIL();
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Message \n    Expected <Hello>\n     but got <World>";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Fail failed", __FILE__, __LINE__);
        }
    }
}

TEST(not_equals)
{
    try
    {
        ASSERT_NOT_EQUALS(1, 2);
    }
    catch (UnitTests::TestFailure& e)
    {
        FAIL();
    }
}

TEST(not_equals1)
{
    try
    {
        ASSERT_NOT_EQUALS(1, 1);
        FAIL();
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Wasn't expecting to get <1>";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Fail failed", __FILE__, __LINE__);
        }
    }
}

TEST(not_equals1_msg)
{
    try
    {
        ASSERT_NOT_EQUALS("Extra message", 1, 1);
        FAIL();
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Extra message Wasn't expecting to get <1>";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Fail failed", __FILE__, __LINE__);
        }
    }
}
