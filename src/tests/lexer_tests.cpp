//
// Created by Lexi Allen on 9/19/2022.
//
#ifndef CHEESE_NO_SELF_TESTS
#include "lexer/lexer.h"
#include "vterm.h"
#include "error.h"
#include "tests/tests.h"
namespace cheese::tests::lexer_tests {
    TEST_SECTION("lexer tests",1)
        TEST_SUBSECTION("token representations")
            TEST_GENERATOR {
                std::uint32_t tBegin = 0;
                auto tEnd = static_cast<std::uint32_t>(cheese::lexer::TokenType::EoF);
                for (std::uint32_t i = tBegin; i <= tEnd; i++) {
                    std::string test_name = "ensuring token ID " + std::to_string(i) + " has a string representation";
                    TEST_GEN_BEGIN(test_name)
                        auto t = static_cast<cheese::lexer::TokenType>(i);
                        try {
                            static_cast<void>(cheese::lexer::name_of(t));
                            TEST_PASS;
                        } catch (std::exception& e) {
                            TEST_FAIL;
                        }
                    TEST_GEN_END
                }
            }
        TEST_END
#define TEST_COMMENT_WARNING(name,error_code) TEST_SUBSECTION(# name " warnings") \
            std::string expected;                                                 \
            TEST_SETUP {                                                               \
                 expected = "expected error: " + getError((error_code));          \
            }                                                                      \
            __TEST_CASE(OFFSET_NAME_1(__LINE__), "single line comments with '" # name "' in them should give a warning",true,true){ \
                TEST_SETUP_ERROR_OUTPUT;                                                                 \
                std::string warning_comment = "//" # name "...";                 \
                 TEST_EXPECT_MESSAGE(cheese::lexer::lex(warning_comment),error_code,expected);\
            }                                                                     \
            TEST_WARN("multi line comments with '" # name "' in them should give a warning"){ \
                 std::string warning_comment = "/*implemented\n" # name "...*/";                 \
                 TEST_EXPECT_MESSAGE(cheese::lexer::lex(warning_comment),error_code,expected);\
            }                                                                          \
        TEST_END
        TEST_SUBSECTION("comment warnings")
            TEST_COMMENT_WARNING(TODO,error::ErrorCode::TodoFoundInComment)
            TEST_COMMENT_WARNING(HACK,error::ErrorCode::HackFoundInComment)
            TEST_COMMENT_WARNING(XXX,error::ErrorCode::XXXFoundInComment)
            TEST_COMMENT_WARNING(BUG,error::ErrorCode::BugFoundInComment)
            TEST_COMMENT_WARNING(FIXME,error::ErrorCode::FixmeFoundInComment)
        TEST_END
#undef TEST_COMMENT_WARNING

        TEST_SUBSECTION("errors for unterminated tokens")
            TEST_ERR("unterminated strings should cause an error") {
                std::string str = "\"abcdef";
                TEST_EXPECT(cheese::lexer::lex(str), error::ErrorCode::UnterminatedStringLiteral);
            }
            TEST_ERR("unterminated character literals should cause an error") {
                std::string str = "\'a";
                TEST_EXPECT(cheese::lexer::lex(str), error::ErrorCode::UnterminatedCharacterLiteral);
            }
            TEST_ERR("unterminated block comments should cause an error") {
                std::string str = "/* this is a comment";
                TEST_EXPECT(cheese::lexer::lex(str), error::ErrorCode::UnterminatedMultilineComment);
            }
            TEST_ERR("unterminated nested block comments should cause an error") {
                std::string str = "/* this is a /* nested comment";
                TEST_EXPECT(cheese::lexer::lex(str), error::ErrorCode::UnterminatedMultilineComment);
            }
        TEST_END

        TEST_SUBSECTION("validation")
            TEST_CASE("lexer should give correct locations") {
                TEST_SETUP_ERROR_OUTPUT;
                std::string loc_str = "a b c d\te\nf g h i j";
                std::vector<std::pair<std::uint32_t,std::uint32_t>> locations{
                        {1,1},
                        {1,3},
                        {1,5},
                        {1,7},
                        {1,9},
                        {1,10},
                        {2,1},
                        {2,3},
                        {2,5},
                        {2,7},
                        {2,9},
                };
                std::vector<cheese::lexer::Token> tokens;
                TEST_TRY(tokens=cheese::lexer::lex(loc_str));
                std::string continue_message = "unexpected amount of tokens: " + std::to_string(tokens.size()) + " expected: " + std::to_string(locations.size() + 1) + '\n';
                TEST_ASSERT_EQ_CONTINUE_MESSAGE(tokens.size(),locations.size()+1,continue_message);
                std::uint32_t i = 0;
                for (auto& token : tokens) {
                    if (token.ty == cheese::lexer::TokenType::EoF) {
                        break;
                    }
                    std::string loc_error_message  = "token #" + std::to_string(i+1) + "'s is " +
                            std::to_string(token.location.line_number) + ':' +
                            std::to_string(token.location.column_number) + ", expected: " +
                            std::to_string(locations[i].first) + ':' + std::to_string(locations[i].second);
                    TEST_ASSERT_CONTINUE_MESSAGE((token.location.line_number = locations[i].first) && (token.location.column_number == locations[i].second), loc_error_message);
                    ++i;
                }
                TEST_PASS;
            }
            TEST_SUBSECTION("numeric literals")
                TEST_GENERATOR {
                    using enum cheese::lexer::TokenType;
                    std::vector<std::tuple<std::string,std::string,cheese::lexer::TokenType>> literals_to_test{
                            {"zero","0",DecimalLiteral},
                            {"long number","1234567890",DecimalLiteral},
                            {"long number with underscores","1_234_567_890", DecimalLiteral},
                            {"binary literal","0b101010",BinaryLiteral},
                            {"binary literal with underscores","0b10_10_10",BinaryLiteral},
                            {"octal literal","0o01234567",OctalLiteral},
                            {"octal literal with underscores", "0o01_23_45_67",OctalLiteral},
                            {"hex literal", "0x0123456789ABCDEF", HexLiteral},
                            {"hex literal with underscores", "0x0123_4567_89ab_cdef", HexLiteral},
                            {"floating point literal", "0.1", FloatingLiteral},
                            {"floating point literal with exponent", "0.1e3", FloatingLiteral},
                            {"floating point literal with negative exponent", "0.1e-3",FloatingLiteral},
                            {"implicit floating point literal", "1e3", FloatingLiteral},
                            {"imaginary literal", "1.4I", ImaginaryLiteral},
                            {"imaginary literal with exponent", "1.4E-3I", ImaginaryLiteral},
                    };

                    for (auto& to_test : literals_to_test) {
                        TEST_GEN_BEGIN(std::get<0>(to_test))
                            TEST_SETUP_ERROR_OUTPUT;
                            std::vector<cheese::lexer::Token> tokens;
                            TEST_TRY(tokens = cheese::lexer::lex(std::get<1>(to_test)));
                            std::string message = "expected: 1 token, got: " + std::to_string(tokens.size()-1) + "\n";
                            TEST_ASSERT_EQ_CONTINUE_MESSAGE(tokens.size(),2,message);
                            message = "expected: " + std::string(cheese::lexer::name_of(std::get<2>(to_test))) + ", got: " + std::string(cheese::lexer::name_of(tokens[0].ty));
                            TEST_ASSERT_EQ_CONTINUE_MESSAGE(std::get<2>(to_test),tokens[0].ty,message);
                            message = "expected: " + std::get<1>(to_test) + ", got: " + std::string(tokens[0].value);
                            TEST_ASSERT_EQ_MESSAGE(std::get<1>(to_test),tokens[0].value,message);
                        TEST_GEN_END
                    }
                }
            TEST_END
            TEST_SUBSECTION("logical modification operators")
                TEST_GENERATOR {
                    using enum cheese::lexer::TokenType;
                    std::vector<std::tuple<std::string,std::string,cheese::lexer::TokenType>> operators_to_test{
                            {"and assign", "and=", AndAssign},
                            {"or assign", "or=", OrAssign},
                            {"xor assign", "xor=", XorAssign},
                    };
                    for (auto& to_test : operators_to_test) {
                        TEST_GEN_BEGIN(std::get<0>(to_test))
                            TEST_SETUP_ERROR_OUTPUT;
                            std::vector<cheese::lexer::Token> tokens;
                            TEST_TRY(tokens = cheese::lexer::lex(std::get<1>(to_test)));
                            std::string message = "expected: 1 token, got: " + std::to_string(tokens.size()-1) + "\n";
                            TEST_ASSERT_EQ_CONTINUE_MESSAGE(tokens.size(),2,message);
                            message = "expected: " + std::string(cheese::lexer::name_of(std::get<2>(to_test))) + ", got: " + std::string(cheese::lexer::name_of(tokens[0].ty));
                            TEST_ASSERT_EQ_CONTINUE_MESSAGE(std::get<2>(to_test),tokens[0].ty,message);
                            message = "expected: " + std::get<1>(to_test) + ", got: " + std::string(tokens[0].value);
                            TEST_ASSERT_EQ_MESSAGE(std::get<1>(to_test),tokens[0].value,message);
                        TEST_GEN_END
                    }
                };
            TEST_END
        TEST_END
    TEST_END
}
#endif