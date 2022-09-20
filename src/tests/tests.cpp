//
// Created by Lexi Allen on 9/19/2022.
//
#include "tests/tests.h"
#include "configuration.h"
#include "vterm.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <fstream>
#include "lexer/lexer.h"
#include "error.h"
#include <sstream>
namespace cheese::tests {
    std::vector<Test*> all_tests;

    void run_all_builtin() {
        std::cout << "RUNNING BUILTIN TESTS\n";
        std::uint32_t pass = 0;
        std::uint32_t fail = 0;
        for (auto test : all_tests) {
            if (test->expects_error) {
                configuration::die_on_first_error = true;
            } else {
                configuration::die_on_first_error = false;
            }
            if (test->expects_warning) {
                configuration::warnings_are_errors = true;
            } else {
                configuration::warnings_are_errors = false;
            }
            std::cout << "Running test: " << test->name << '\n';
            bool result = true;
            try {
                result = test->test();
            } catch (std::exception& e) {
                std::cerr << e.what();
                result = false;
            }
            if (result) {
                pass += 1;
                if (configuration::use_escape_sequences) {
                    io::setForegroundColor(io::Color::Green);
                }
                std::cout << "\t[PASS]\n";
                if (configuration::use_escape_sequences) {
                    io::setForegroundColor(io::Color::Gray);
                }
            } else {
                fail += 1;
                if (configuration::use_escape_sequences) {
                    io::setForegroundColor(io::Color::Red);
                }
                std::cout << "\t[FAIL]\n";
                if (configuration::use_escape_sequences) {
                    io::setForegroundColor(io::Color::Gray);
                }
            }
        }
        std::uint32_t total_ran = pass+fail;
        std::cout << "Results:\n\tTests ran: " << total_ran << "\n\tSuccess: " << pass << "\n\tFailure: " << fail << "\n";
    }
    void run_json_tests(std::string filename) {
        std::cout << "Running tests from: " << filename << '\n';
        std::string json_buffer;
        try {
            std::ifstream inf(filename);
            std::stringstream buffer;
            buffer << inf.rdbuf();
            json_buffer = buffer.str();
        } catch (std::exception e) {
            std::cout << "Failed to run tests due to error opening file\n";
            return;
        }
        auto object = nlohmann::json::parse(json_buffer);
        if (object.is_array()) {
            std::uint32_t pass = 0;
            std::uint32_t fail = 0;
            for (auto subobject : object) {
                bool result = false;
                try {
                    result = run_single_json_test(subobject);
                } catch (std::exception& e) {
                    std::cout << '\t' << e.what() << '\n';
                    ++fail;
                }
                if (result) {
                    pass += 1;
                    if (configuration::use_escape_sequences) {
                        io::setForegroundColor(io::Color::Green);
                    }
                    std::cout << "\t[PASS]\n";
                    if (configuration::use_escape_sequences) {
                        io::setForegroundColor(io::Color::Gray);
                    }
                } else {
                    fail += 1;
                    if (configuration::use_escape_sequences) {
                        io::setForegroundColor(io::Color::Red);
                    }
                    std::cout << "\t[FAIL]\n";
                    if (configuration::use_escape_sequences) {
                        io::setForegroundColor(io::Color::Gray);
                    }
                }
            }
            std::uint32_t total_ran = pass+fail;
            std::cout << "Results:\n\tTests ran: " << total_ran << "\n\tSuccess: " << pass << "\n\tFailure: " << fail << "\n";
        } else {
            std::cout << "Expected file to contain an array of JSON tests\n";
        }
    }
    bool run_single_json_test(nlohmann::json json_test) {
        if (!json_test.is_object()) {
            std::cout << "\tTest must be an object\n";
            return false;
        }
        auto name = json_test["test_name"];
        if (!name.is_string()) {
            std::cout << "\tTest must have a name that is a string\n";
            return false;
        }
        auto name_str = name.get<std::string>();
        std::cout << "Running test: " << name_str << '\n';
        auto type = json_test["test_type"];
        if (!type.is_string()) {
            std::cout << "\tTest must have a type that is a string\n";
            return false;
        }

        auto type_str = type.get<std::string>();
        if (type_str == "standalone") {
            auto location = json_test["test_location"];
            if (!location.is_string()) {
                std::cout << "\tTest location must be a string\n";
                return false;
            }
            auto filename = location.get<std::string>();
            std::string file_buffer;
            try {
                std::ifstream inf(filename);
                std::stringstream buffer;
                buffer << inf.rdbuf();
                file_buffer = buffer.str();
            } catch (std::exception e) {
                std::cout << "\tCould not read '" << filename << "'\n";
                return false;
            }
            std::cout << "opened " << filename << "...\n";
            nlohmann::json stages = json_test["test_stages"];
            if (!stages.is_object()) {
                std::cout << "\tTest stages must be an object\n";
                return false;
            }
            bool has_lexer_info = stages.contains("lexer");
            bool has_parser_info = stages.contains("parser");
            bool has_bacteria_info = stages.contains("bacteria");
            bool has_llvm_info = stages.contains("llvm");
            bool has_build_info = stages.contains("build");
            auto should_llvm = has_llvm_info || has_build_info;
            auto should_bacteria = has_bacteria_info || should_llvm;
            auto should_parse = has_parser_info || should_bacteria;
            std::vector<cheese::lexer::Token> tokens;
            if (has_lexer_info) {
                //Do more validation here
                auto lexer_info = stages["lexer"];
                auto expect = lexer_info["expect"];
                auto expectation = expect["expectation"].get<std::string>();
                if (expectation == "success") {
                    std::cout << "\tchecking if lexer succeeds...\n";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        std::cout << "\tlexer succeeded...\n";
                    } catch (std::exception& e){
                        std::cout << '\t' << e.what() << '\n';
                        return false;
                    }
                } else if (expectation == "error") {
                    configuration::die_on_first_error = true;
                    auto expected_code = expect["code"].get<std::uint32_t>();
                    std::cout << "\tchecking if lexer gives error " << error::getError(expected_code) << "...\n";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        configuration::die_on_first_error = false;
                        std::cout << "\tlexer did not error\n";
                        return false;
                    } catch (cheese::error::CompilerError& compilerError) {
                        if (static_cast<std::uint32_t>(compilerError.code) != expected_code) {
                            std::cout << "\tgot error code " << error::getError(compilerError.code) << ", expected: " << error::getError(expected_code) << '\n';
                            std::cout << '\t' << compilerError.what() << '\n';
                            configuration::die_on_first_error = false;
                            return false;
                        } else {
                            std::cout << "\tlexer gave error " << error::getError(expected_code) << "\n";
                            configuration::die_on_first_error = false;
                            return true;
                        }
                    } catch (std::exception& e) {
                        std::cout << '\t' << e.what() << '\n';
                        configuration::die_on_first_error = false;
                        return false;
                    }
                } else if (expectation == "warn") {
                    configuration::die_on_first_error = true;
                    configuration::warnings_are_errors = true;
                    auto expected_code = expect["code"].get<std::uint32_t>();
                    std::cout << "\tchecking if lexer gives warning " << error::getError(expected_code) << "...\n";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        configuration::die_on_first_error = false;
                        configuration::warnings_are_errors = false;
                        std::cout << "\tlexer did not warn\n";
                        return false;
                    } catch (cheese::error::CompilerError& compilerError) {
                        if (static_cast<std::uint32_t>(compilerError.code) != expected_code) {
                            std::cout << "\tgot code " << error::getError(compilerError.code) << ", expected: " << error::getError(expected_code) << '\n';
                            std::cout << '\t' << compilerError.what() << '\n';
                            configuration::die_on_first_error = false;
                            configuration::warnings_are_errors = false;
                            return false;
                        } else {
                            std::cout << "\tlexer gave warning " << error::getError(expected_code) << "\n";
                            configuration::die_on_first_error = false;
                            configuration::warnings_are_errors = false;
                            return true;
                        }
                    } catch (std::exception& e) {
                        std::cout << '\t' << e.what() << '\n';
                        configuration::die_on_first_error = false;
                        configuration::warnings_are_errors = false;
                        return false;
                    }
                } else if (expectation == "match") {
                    auto expected_pattern = expect["pattern"].get<std::string>();
                    std::cout << "\tchecking if lexer matches pattern...\n";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        auto lexer_pattern = cheese::lexer::to_stream(tokens);
                        if (lexer_pattern != expected_pattern) {
                            std::cout << "\tgot incorrect pattern:\n";
                            std::cout << "\t\t" << lexer_pattern << '\n';
                            std::cout << "\texpected:\n";
                            std::cout << "\t\t" << expected_pattern << '\n';
                            return false;
                        } else {
                            std::cout << "\tlexer gave correct pattern...\n";
                        }
                    } catch (std::exception& e){
                        std::cout << '\t' << e.what() << '\n';
                        return false;
                    }
                }
                if (lexer_info["output"].get<bool>()) {
                    std::cout << "\tgiving lexer output...\n";
                    std::cout << "\tlexer output: " << cheese::lexer::to_stream(tokens) << '\n';
                }
            } else {
                std::cout << "\timplicitly checking if lexer succeeds...\n";
                try {
                    tokens = cheese::lexer::lex(file_buffer,filename);
                    std::cout << "\tlexer succeeded...\n";
                } catch (std::exception& e) {
                    std::cout << e.what();
                    return false;
                }
            }
            if (should_parse) {
                std::cout << "\tparsing not yet implemented\n";
                return false;
            }
        } else {
            std::cout << "Unimplemented test type: " << type_str << "\n";
            return false;
        }
        return true;
    }

    void gen_pass() {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Green);
        }
        std::cout << "[PASS]";
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    void gen_fail() {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Red);
        }
        std::cout << "[FAIL]\n";
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }

    }
}