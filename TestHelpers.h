
#if !defined(TestFramework_TestHelpers_h_)
#define TestFramework_TestHelpers_h_
#include <string>

namespace UnitTests
{
    // you can use these with PARAM_TEST, e.g. :
    //               UnitTests::in_out<std::string> data[] = { {"a", "A", } };
    //               PARAM_TEST(test, data)
    //               {
    //                  ASSERT_EQUAL(param.out, img::toupper(param.in));
    //               }
    //  or msg_in_out :
    //               UnitTests::msg_in_out<std::string> data[] = { {"single_character", "a", "A", } };
    //               PARAM_TEST(test, data)
    //               {
    //                  ASSERT_EQUAL(param.msg, param.out, img::toupper(param.in));
    //               }
    //  or binary_in_out :
    //               UnitTests::binary_in_out<int> data[] = { {1, 2, 3, } };
    //               PARAM_TEST(test, data)
    //               {
    //                  ASSERT_EQUAL(param.out, param.lhs + param.rhs);
    //               }
    //  or msg_binary_in_out :
    //               UnitTests::msg_binary_in_out<int> data[] = { {"positive", 1, 2, 3, } };
    //               PARAM_TEST(test, data)
    //               {
    //                  ASSERT_EQUAL(param.msg, param.out, param.lhs + param.rhs);
    //               }
    //
    template<typename In, typename Out = In>
    struct in_out
    {
        In  in;
        Out out;
    };

    template<typename In, typename Out = In>
    struct msg_in_out
    {
        std::string msg;
        In          in;
        Out         out;
    };
    template<typename Lhs, typename Rhs = Lhs, typename Out = Lhs>
    struct binary_in_out
    {
        Lhs lhs;
        Rhs rhs;
        Out out;
    };

    template<typename Lhs, typename Rhs = Lhs, typename Out = Lhs>
    struct msg_binary_in_out
    {
        std::string msg;
        Lhs         lhs;
        Rhs         rhs;
        Out         out;
    };
} // namespace UnitTests

#endif
