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

namespace cheese::tests::parser_tests {
    TEST_SECTION("parser", 1)
        TEST_SUBSECTION("generated tests")
            //This should contain the minimized and deflated json in the parser_tests.json file in this same directory.
            const char* all_parser_generated_tests =
                "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";

            const size_t generated_tests_max_size = 32768;
            nlohmann::json generated_tests_json;
            TEST_SETUP {
                Decompressor dc = Decompressor();
                auto input_stream_pair =compression::base64_decode(all_parser_generated_tests);
                std::uint8_t* input_stream = input_stream_pair.first;
                size_t input_size = input_stream_pair.second;
                std::uint8_t* output_stream = new std::uint8_t[generated_tests_max_size+1];
                size_t output_size = dc.Feed(input_stream,input_size,output_stream, generated_tests_max_size, true);
                output_stream[output_size] = 0;
                generated_tests_json = nlohmann::json::parse(output_stream);
                delete[] output_stream;
                delete[] input_stream;
            }
            TEST_SUBSECTION("parser validation")
                TEST_GENERATOR {
                    for (size_t i = 0; i < generated_tests_json.size(); i++) {
                        auto test = generated_tests_json[i];
                        TEST_GEN_BEGIN(test[0].get<std::string>())
                            std::vector<lexer::Token> tokens;
                            TEST_TRY(tokens = lexer::lex(test[1].get<std::string>()));
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