//
// Created by Lexi Allen on 9/19/2022.
//
#ifndef CHEESE_NO_SELF_TESTS
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
#include <unordered_map>
#include <future>
#include <mutex>
namespace cheese::tests {
    std::unordered_map<std::uint32_t,TestSection*> all_test_sections;
    std::vector<std::uint32_t> all_priorities;
    void run_all_builtin() {
//        configuration::log_errors = false;
        configuration::log_errors = true;
//        configuration::use_escape_sequences = false;
        std::sort(all_priorities.begin(),all_priorities.end());
        TestResults results;
        for (auto prio : all_priorities) {
            auto section = all_test_sections[prio];
            std::cout << "section: " << section->section_name << '\n';
            results = results + section->run(1);
        }
        results.display_results(0);
        configuration::log_errors = true;
    }
    void run_json_tests(std::string filename) {
        configuration::log_errors = false;
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
        configuration::log_errors = true;
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
                    std::cout << "\tchecking if lexer succeeds...";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        gen_pass();
                        std::cout << '\n';
                    } catch (std::exception& e){
                        gen_fail();
                        std::cout << '\t' << e.what() << '\n';
                        return false;
                    }
                } else if (expectation == "error") {
                    configuration::die_on_first_error = true;
                    auto expected_code = expect["code"].get<std::uint32_t>();
                    std::cout << "\tchecking if lexer gives error " << error::getError(expected_code) << "...";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        configuration::die_on_first_error = false;
                        gen_fail();
                        return false;
                    } catch (cheese::error::CompilerError& compilerError) {
                        if (static_cast<std::uint32_t>(compilerError.code) != expected_code) {
                            gen_fail();
                            std::cout << "\tgot error code " << error::getError(compilerError.code) << ", expected: " << error::getError(expected_code) << '\n';
                            std::cout << '\t' << compilerError.what() << '\n';
                            configuration::die_on_first_error = false;
                            return false;
                        } else {
                            gen_pass();
                            configuration::die_on_first_error = false;
                            return true;
                        }
                    } catch (std::exception& e) {
                        gen_fail();
                        std::cout << '\t' << e.what() << '\n';
                        configuration::die_on_first_error = false;
                        return false;
                    }
                } else if (expectation == "warn") {
                    configuration::die_on_first_error = true;
                    configuration::warnings_are_errors = true;
                    auto expected_code = expect["code"].get<std::uint32_t>();
                    std::cout << "\tchecking if lexer gives warning " << error::getError(expected_code) << "...";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        configuration::die_on_first_error = false;
                        configuration::warnings_are_errors = false;
                        gen_fail();
                        return false;
                    } catch (cheese::error::CompilerError& compilerError) {
                        if (static_cast<std::uint32_t>(compilerError.code) != expected_code) {
                            gen_fail();
                            std::cout << "\tgot code " << error::getError(compilerError.code) << ", expected: " << error::getError(expected_code) << '\n';
                            std::cout << '\t' << compilerError.what() << '\n';
                            configuration::die_on_first_error = false;
                            configuration::warnings_are_errors = false;
                            return false;
                        } else {
                            gen_pass();
                            configuration::die_on_first_error = false;
                            configuration::warnings_are_errors = false;
                            return true;
                        }
                    } catch (std::exception& e) {
                        gen_fail();
                        std::cout << '\t' << e.what() << '\n';
                        configuration::die_on_first_error = false;
                        configuration::warnings_are_errors = false;
                        return false;
                    }
                } else if (expectation == "match") {
                    auto expected_pattern = expect["pattern"].get<std::string>();
                    std::cout << "\tchecking if lexer matches pattern...";
                    try {
                        tokens = cheese::lexer::lex(file_buffer,filename);
                        auto lexer_pattern = cheese::lexer::to_stream(tokens);
                        if (lexer_pattern != expected_pattern) {
                            gen_fail();
                            std::cout << "\tgot incorrect pattern:\n";
                            std::cout << "\t\t" << lexer_pattern << '\n';
                            std::cout << "\texpected:\n";
                            std::cout << "\t\t" << expected_pattern << '\n';
                            return false;
                        } else {
                            gen_pass();
                        }
                    } catch (std::exception& e){
                        gen_fail();
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
                    gen_pass();
                } catch (std::exception& e) {
                    gen_fail();
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

    void gen_pass(std::uint32_t nesting) {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Green);
        }
        test_output_message(nesting,"[PASS]\n");
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    void gen_fail(std::uint32_t nesting) {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Red);
        }
        test_output_message(nesting,"[FAIL]\n");
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    void gen_skip(std::uint32_t nesting) {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Blue);
        }
        test_output_message(nesting,"[SKIP]\n");
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    std::vector<std::string> internal_split(std::string message) {
        std::uint32_t beg = 0;
        std::uint32_t count = 0;
        std::vector<std::string> res;
        while (beg+count < message.size()) {
            if (message[beg+count] == '\n') {
                res.push_back(message.substr(beg,count+1));
                beg = beg+count+1;
                count = 0;
            } else {
                ++count;
            }
        }
        if (count > 0) res.push_back(message.substr(beg,count));
        return res;
    }

    void test_output_message(std::uint32_t nesting, std::string message) {
        static bool start_new_indentation = true;
        static std::mutex mut;
        mut.lock();
        auto split = internal_split(message);
        for (auto& line : split) {
            if (start_new_indentation) {
                for (std::uint32_t tabs = 0; tabs < nesting; tabs++) {
                    std::cout << '\t';
                }
            }
            std::cout << line;
            start_new_indentation = line[line.size()-1] == '\n';
        }
        mut.unlock();
    }

    void TestResults::display_results(std::uint32_t nesting) {
        test_output_message(nesting, "results:\n");
        test_output_message(nesting+1, "total amount of tests: ");
        test_output_message(nesting+1,std::to_string(pass + fail + skip) + "\n");
        test_output_message(nesting+1,"passed: ");
        test_output_message(nesting+1,std::to_string(pass)+"\n");
        test_output_message(nesting+1,"failed: ");
        test_output_message(nesting+1,std::to_string(fail)+"\n");
        test_output_message(nesting+1,"skipped: ");
        test_output_message(nesting+1,std::to_string(skip)+"\n");
        test_output_message(nesting+1, "percentage: ");
        test_output_message(nesting+1,std::to_string(static_cast<std::uint32_t>( (static_cast<double>(pass)/static_cast<double>(pass+fail+skip)) * 100)) + "%\n");
    }

    TestResults SectionMember::run(int nesting) {
        if (is_subsection) {
            test_output_message(nesting, "section: " + subsection->section_name + "\n");
            return subsection->run(nesting+1);
        } else {
            if (test_case->expects_error) {
                configuration::die_on_first_error = true;
            } else {
                configuration::die_on_first_error = false;
            }
            if (test_case->expects_warning) {
                configuration::warnings_are_errors = true;
            } else {
                configuration::warnings_are_errors = false;
            }
            test_output_message(nesting, test_case->description + "\n");
            return test_case->run(nesting+1);
        }
    }

    TestResults TestSection::run(int nesting) {
        if (setup.has_value()) {
            setup.value()();
        }
        TestResults results;
        for (auto& member : members) {
            results = results + member.run(nesting);
        }
        results.display_results(nesting);
        if (destroy.has_value()) {
            destroy.value()();
        }
        return results;
    }

    TestSection::TestSection(std::string name, std::uint32_t priority) {
        section_name = name;
        all_test_sections[priority] = this;
        all_priorities.push_back(priority);
    }

    TestSection::TestSection(std::string name, TestSection *parent) {
        section_name = name;
        parent->add(this);
    }

    void TestSection::add(TestSection *subsection) {
        SectionMember mem{.is_subsection = true,.subsection = subsection};
        members.push_back(mem);
    }

    void TestSection::add(TestCase *subcase) {
        SectionMember mem{.is_subsection = false,.test_case = subcase};
        members.push_back(mem);
    }

    TestResults TestCase::run(int nesting) {
        try {
//            auto res = test_case(nesting);
            std::packaged_task<SingleResult(int)> task(test_case);
            auto future = task.get_future();
            std::thread thr(std::move(task),nesting);
            SingleResult res = SingleResult::Skip;
            if (future.wait_for(std::chrono::milliseconds(1000)) != std::future_status::timeout) {
                thr.join();
                res = future.get();
            } else {
                thr.detach();
                test_output_message(nesting,"\n");
                test_output_message(nesting,"Skipping due to timeout ");
                gen_skip();
            }
            switch (res) {
                case SingleResult::Pass:
                    return TestResults{1,0,0};
                case SingleResult::Fail:
                    return TestResults{0,1,0};
                case SingleResult::Skip:
                    return TestResults{0,0,1};
            }

        } catch (error::CompilerError& c) {
            gen_fail();
            test_output_message(nesting,c.what() + "\n");
            return TestResults{0,1,0};
        } catch (std::exception& e) {
            gen_fail();
            test_output_message(nesting,e.what());
            test_output_message(nesting,"\n");
            return TestResults{0,1,0};
        }
        return TestResults{0,0,0};
    }

    TestCase::TestCase(TestSection *parent, std::string description, TestCaseWithNesting test_case, bool expects_error,
                       bool expects_warning) : description(description), test_case(test_case), expects_error(expects_error), expects_warning(expects_warning) {
        parent->add(this);
    }
}
#endif