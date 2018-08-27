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
        FAIL("ASSERT_EQUALS should not have fired.");
    }
}

TEST(equals1)
{
    try
    {
        ASSERT_EQUALS("Hello", "World");
        FAIL("ASSERT_EQUALS should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg = e.what();
        std::string expected =
            "error A1000: Assertion failure : \n    Expected <Hello (uncoerced=Hello)>\n     but got "
            "<World (uncoerced=World)>";

        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from failing ASSERT_EQUALS", __FILE__, __LINE__);
        }
    }
}

TEST(equals_msg)
{
    try
    {
        ASSERT_EQUALS("Message", "Hello"s, "World"s);
        FAIL("ASSERT_EQUALS should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Message \n    Expected <Hello>\n     but got <World>";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from failing ASSERT_EQUALS", __FILE__, __LINE__);
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
        FAIL("ASSERT_NOT_EQUALS should not have fired here.");
    }
}

TEST(not_equals1)
{
    try
    {
        ASSERT_NOT_EQUALS(1, 1);
        FAIL("ASSERT_NOT_EQUALS should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Wasn't expecting to get <1>";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from ASSERT_NOT_EQUALS", __FILE__, __LINE__);
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
            throw UnitTests::TestFailure("Unexpected message from failing ASSERT_EQUALS", __FILE__, __LINE__);
        }
    }
}

TEST(assert_true)
{
    try
    {
        ASSERT_TRUE(1 == 1);
    }
    catch (UnitTests::TestFailure& e)
    {
        FAIL("ASSERT_TRUE should not have fired here.");
    }
}

TEST(assert_true1)
{
    try
    {
        ASSERT_TRUE(1 == 2);
        FAIL("ASSERT_TRUE should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure :  Expression evaluated to false";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from ASSERT_TRUE", __FILE__, __LINE__);
        }
    }
}

TEST(assert_true_msg)
{
    try
    {
        ASSERT_TRUE("Extra message", 1 == 2);
        FAIL();
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Extra message Expression evaluated to false";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from failing ASSERT_TRUE", __FILE__, __LINE__);
        }
    }
}

TEST(assert_false)
{
    try
    {
        ASSERT_FALSE(1 == 2);
    }
    catch (UnitTests::TestFailure& e)
    {
        FAIL("ASSERT_FALSE should not have fired here.");
    }
}

TEST(assert_false1)
{
    try
    {
        ASSERT_FALSE(1 == 1);
        FAIL("ASSERT_FALSE should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure :  Expression evaluated to true";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from ASSERT_TRUE", __FILE__, __LINE__);
        }
    }
}

TEST(assert_false_msg)
{
    try
    {
        ASSERT_FALSE("More Info", 1 == 1);
        FAIL("ASSERT_FALSE should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : More Info Expression evaluated to true";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from ASSERT_TRUE", __FILE__, __LINE__);
        }
    }
}

TEST(assert_in)
{
    try
    {
        std::vector<int> v{0, 1, 2, 3};
        ASSERT_IN(1, v);
    }
    catch (UnitTests::TestFailure& e)
    {
        FAIL("ASSERT_IN should not have fired here.");
    }
}

TEST(assert_in1)
{
    try
    {
        std::vector<int> v{0, 1, 2, 3};
        ASSERT_IN(10, v);
        FAIL("ASSERT_IN should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg      = e.what();
        std::string expected = "error A1000: Assertion failure : Expected container to contain 10, actual contents :\n"
                               "\t[0, 1, 2, 3]";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from ASSERT_TRUE", __FILE__, __LINE__);
        }
    }
}

TEST(assert_in_msg)
{
    try
    {
        std::vector<int> v{0, 1, 2, 3};
        ASSERT_IN("Extra message", 10, v);
        FAIL("ASSERT_IN should have fired here.");
    }
    catch (UnitTests::TestFailure& e)
    {
        std::string msg = e.what();
        std::string expected =
            "error A1000: Assertion failure : Extra message. Expected container to contain 10, actual contents :\n"
            "\t[0, 1, 2, 3]";
        if (msg.find(expected) == std::string::npos)
        {
            throw UnitTests::TestFailure("Unexpected message from ASSERT_TRUE", __FILE__, __LINE__);
        }
    }
}
