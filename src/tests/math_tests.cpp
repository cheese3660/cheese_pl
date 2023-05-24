//
// Created by Lexi Allen on 5/24/2023.
//
#ifndef CHEESE_NO_SELF_TESTS
#include "math/BigInteger.h"
#include "vterm.h"
#include "error.h"
#include "tests/tests.h"
using bigint = cheese::math::BigInteger;
namespace cheese::tests::math {
    TEST_SECTION("BigInteger tests",0)
        TEST_SUBSECTION("Creation")
            TEST_CASE("Default initialization is zero") {
                bigint zero;
                TEST_ASSERT(zero.zero());
            }
            TEST_CASE("Zero") {
                bigint zero = 0;
                TEST_ASSERT(zero.zero());
            }
            TEST_CASE("One") {
                bigint one = 1;
                TEST_ASSERT(one == 1);
            }
            TEST_CASE("Two") {
                bigint two = 2;
                TEST_ASSERT(two == 2);
            }
            TEST_CASE("5 billion") {
                bigint fbil{"5000000000"};
                TEST_ASSERT(fbil == "5000000000");
            }
            TEST_CASE("69 Sextillion") {
                bigint sx{"69000000000000000000000"};
                TEST_ASSERT(sx == "69000000000000000000000");
            }
            TEST_CASE("Negative one") {
                bigint negative_one = -1;
                TEST_ASSERT(negative_one == -1);
            }
            TEST_CASE("Negative five billion") {
                bigint negative_five_bill{"-5000000000"};
                TEST_ASSERT(negative_five_bill == "-5000000000");
            }
            TEST_CASE("Negative 69 sextillion") {
                bigint negative_69_sx{"-69000000000000000000000"};
                TEST_ASSERT(negative_69_sx == "-69000000000000000000000");
            }

        TEST_END
        TEST_SUBSECTION("Comparison Operators")
            bigint zero;
            bigint one = 1;
            bigint negative_one = -1;
            bigint two = 2;
            bigint fbil{"5000000000"};
            bigint sx{"69000000000000000000000"};
            bigint negative_five_bill{"-5000000000"};
            bigint negative_69_sx{"-69000000000000000000000"};
            TEST_SUBSECTION("Between 2 big ints")
                TEST_CASE("0 < 1") {
                    TEST_ASSERT(zero < one);
                }
                TEST_CASE("0 < 2") {
                    TEST_ASSERT(zero < two);
                }
                TEST_CASE("0 < 5,000,000,000") {
                    TEST_ASSERT(zero < fbil);
                }
                TEST_CASE("0 < 69,000,000,000,000,000,000,000") {
                    TEST_ASSERT(zero < sx);
                }
                TEST_CASE("0 > -1") {
                    TEST_ASSERT(zero > negative_one);
                }
                TEST_CASE("0 > -5,000,000,000") {
                    TEST_ASSERT(zero > negative_five_bill);
                }
                TEST_CASE("0 > -69,000,000,000,000,000,000,000") {
                    TEST_ASSERT(zero > negative_69_sx);
                }
                TEST_CASE("-69,000,000,000,000,000,000,000 < 69,000,000,000,000,000,000,000") {
                    TEST_ASSERT(negative_69_sx < sx);
                }
            TEST_END
            TEST_SUBSECTION("Between big ints and literals")
                TEST_CASE("0 < 1") {
                    TEST_ASSERT(zero < 1);
                }
                TEST_CASE("0 < 2") {
                    TEST_ASSERT(zero < 2);
                }
                TEST_CASE("0 > -1") {
                    TEST_ASSERT(zero > -1);
                }
                TEST_CASE("0 > -69,000,000,000,000,000,000,000") {
                    TEST_ASSERT(zero > "-69,000,000,000,000,000,000,000");
                }
            TEST_END

        TEST_END

        TEST_SUBSECTION("Addition/Subtraction")
            bigint zero;
            bigint one = 1;
            bigint negative_one = -1;
            bigint two = 2;
            bigint fbilm2{"4999999998"};
            bigint fbil{"5000000000"};
            bigint fbilp2{"5000000002"};
            bigint sx{"69000000000000000000000"};
            bigint negative_five_bill{"-5000000000"};
            bigint negative_69_sx{"-69000000000000000000000"};

            TEST_SUBSECTION("Between 2 big ints")
                TEST_CASE("0 + 1 == 1") {
                    auto sum = zero + one;
                    TEST_ASSERT_MESSAGE(sum == one, "0 + 1 != " + static_cast<std::string>(sum));
                }
                TEST_CASE("0 - 1 == -1") {
                    auto sum = zero - one;
                    TEST_ASSERT_MESSAGE(sum == negative_one, "0 - 1 != " + static_cast<std::string>(sum));
                }
                TEST_CASE("2 + -1 == 1") {
                    auto sum = two + negative_one;
                    TEST_ASSERT_MESSAGE(sum == one, "2 + -1 != " + static_cast<std::string>(sum));
                }
                TEST_CASE("5,000,000,000 + 2 = 5,000,000,002") {
                    auto sum = fbil + two;
                    TEST_ASSERT_MESSAGE(sum == fbilp2, "5,000,000,000 + 2 != " + static_cast<std::string>(sum));
                }
            TEST_END
            TEST_SUBSECTION("Between big ints and literals")
            TEST_END
        TEST_END

        TEST_SUBSECTION("Multiplication/Division")
        TEST_END

        TEST_SUBSECTION("Bitwise Operators")
        TEST_END

        TEST_SUBSECTION("Casting")
            TEST_CASE("static_cast<std::string>(32) == \"32\"") {
                bigint num = 32;
                auto cast = static_cast<std::string>(num);
                TEST_ASSERT_MESSAGE(cast == "32", cast + '\n');
            }
            TEST_CASE("static_cast<std::string>(-366992) == \"-366992\"") {
                bigint num = -366992;
                auto cast = static_cast<std::string>(num);
                TEST_ASSERT_MESSAGE(cast == "-366992", cast + '\n');
            }
        TEST_END
    TEST_END
}
#endif