//
// Created by Lexi Allen on 5/27/2023.
//

#include "tests/tests.h"
#include "bacteria/BacteriaNode.h"
#include "bacteria/BacteriaType.h"
#include "error.h"
#include "fstream"
#include "sstream"
#include "filesystem"
#include "parser/Node.h"
#include "parser/parser.h"
#include "curdle/curdle.h"
#include "util/json_template.h"
#include <iostream>
#ifndef CHEESE_NO_SELF_TESTS
namespace cheese::tests::curdle_tests {
    struct TempFile {
        std::string name;
        TempFile(std::string name, std::string data) : name(name) {
            std::ofstream f{name};
            f << data;
        }
        ~TempFile() {
            std::filesystem::remove(name);
        }
    };
    TEST_SECTION("curdle", 3)
        TEST_SUBSECTION("generated tests")
            nlohmann::json generated_tests_json;
            TEST_SETUP {
                std::ifstream curdle_tests{"./curdle_tests.json"}; //Make it local to an environment variable when releasing
                std::stringstream buf;
                buf << curdle_tests.rdbuf();
                generated_tests_json = nlohmann::json::parse(buf);
                util::templatify(generated_tests_json,"./templates");
            }
            TEST_SUBSECTION("curdle validation")
                TEST_GENERATOR {
                    for (auto test : generated_tests_json) {
                        try {
                            TEST_GEN_BEGIN(test[0].get<std::string>())
                                configuration::error_output_handler = [__nesting](std::string msg) {
                                    test_output_message(__nesting,msg);
                                };
                                std::vector<lexer::Token> tokens;
                                std::string fname;
                                std::string str;
                                std::unique_ptr<TempFile> captured_temp;
                                if (test[1].get<std::string>().ends_with(".chs")) {
                                    fname = "./testenv_src/" + test[1].get<std::string>();
                                    std::ifstream file{fname};
                                    std::stringstream buf;
                                    TEST_TRY(buf << file.rdbuf());
                                    str = buf.str();
                                } else {
                                    fname = "./__testing_temp_file__";
                                    str = test[1].get<std::string>();
                                    captured_temp = std::make_unique<TempFile>(str,fname);
                                }
                                TEST_TRY(tokens=lexer::lex(str,fname));
                                std::shared_ptr<parser::Node> root;
                                TEST_TRY(root = parser::parse(tokens));
                                bacteria::BacteriaPtr bact;
                                TEST_TRY(bact = cheese::curdle::curdle(cheese::curdle::Project{
                                    "./testenv_src/",
                                    {"./testenv_imports/"},
                                    fname,
                                    root,
                                    cheese::curdle::ProjectType::Application
                                }));
                                auto fail_message = "got:\n" + bact->as_json().dump(1) + "\nin text:\n" + bact->get_textual_representation() + "\nexpected:\n" + test[2].dump(1) + "\n";
                                TEST_ASSERT_MESSAGE(bact->compare_json(test[2]),fail_message);
                            TEST_GEN_END
                        } catch (const std::exception& e) {
                            throw;
                        }
                    }
                }
            TEST_END
        TEST_END
    TEST_END
}
#endif