//
// Created by Lexi Allen on 9/21/2022.
//
#ifndef CHEESE_NO_SELF_TESTS
#include "tests/tests.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "thirdparty/json.hpp"
#include "../../external/inflatecpp/decompressor.h"
#include "compression/base64.h"
#include "iostream"
#include "error.h"
#include "fstream"
#include "sstream"

namespace cheese::tests::parser_tests {
    TEST_SECTION("parser", 1)
        TEST_SUBSECTION("generated tests")
            //This should contain the minimized and deflated json in the parser_tests.json file in this same directory.
            nlohmann::json generated_tests_json;
            TEST_SETUP {
                std::ifstream parser_tests{"./parser_tests.json"}; //Make it local to an environment variable when releasing
                std::stringstream buf;
                buf << parser_tests.rdbuf();
                generated_tests_json = nlohmann::json::parse(buf);
            }
            TEST_SUBSECTION("parser validation")
                TEST_GENERATOR {
                    for (size_t i = 0; i < generated_tests_json.size(); i++) {
                        auto test = generated_tests_json[i];
                        TEST_GEN_BEGIN(test[0].get<std::string>())
                            std::vector<lexer::Token> tokens;
                            auto buffer = test[1].get<std::string>();
                            TEST_TRY(tokens = lexer::lex(buffer));
                            //Lets for the sake of testing output the lexed stream
//                            std::cout << '\n';
//                            test_output_message(__nesting,lexer::to_stream(tokens));
                            std::shared_ptr<parser::Node> root;
                            TEST_TRY(root = parser::parse(tokens));
                            auto fail_message = "got:\n" + root->as_json().dump(1) + "\nexpected:\n" + test[2].dump(1) + "\n";
                            TEST_ASSERT_MESSAGE(root->compare_json(test[2]),fail_message);
                        TEST_GEN_END
                    }
                }
            TEST_END
        TEST_END
        TEST_SUBSECTION("parser errors")
            TEST_GENERATOR {
            }
        TEST_END
    TEST_END
}
#endif