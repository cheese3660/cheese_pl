#include <iostream>
#include "lexer/lexer.h"
#include "NotImplementedException.h"
#include "vterm.h"
#include <argparse/argparse.hpp>
#include "configuration.h"
#include <fstream>
const char* name = "cheese";
const char* version = "0.0.1";
const char* copyright = "(C) Lexi Allen";
int main(int argc, const char** argv) {
    argparse::ArgumentParser program(name,version,argparse::default_arguments::all);
    std::string program_mode = "help";
    program.add_argument("mode")
        .help("the mode of the program")
        .default_value<std::string>("help")
        .action([&](const std::string& mode) {program_mode = mode;})
        .required();
    program.add_argument("--no-vterm")
        .help("disables virtual terminal escapes for errors and the like")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);
    program.add_argument("--lex")
        .help("test the lexer (mode = 'test' only)")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);
    program.add_argument("--parse")
        .help("test the parser (mode = 'test' only)")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);
    program.add_argument("--bacteria")
        .help("test bacteria (mode = 'test' only)")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);
    program.add_argument("--llvm")
        .help("test the LLVM ir generation (mode = 'test' only)")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("...")
        .help("various input files, meaning changes depending on the mode")
        .default_value<std::vector<std::string>>({})
        .append()
        .nargs(argparse::nargs_pattern::any);


    program.add_epilog("Possible modes are:\n\t'test' (used for testing the compiler)");
    //More args to be added later, maybe at some point redo this


    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << '\n';
        std::cerr << program;
        std::exit(1);
    }
    cheese::configuration::use_escape_sequences = !program.get<bool>("--no-vterm");
    if (program_mode == "help") {
        std::cout << program;
    } else if (program_mode == "test") {
        auto test_lexer = program.get<bool>("--lex");
        auto test_parser = program.get<bool>("--parse");
        auto test_bacteria = program.get<bool>("--bacteria");
        auto test_llvm = program.get<bool>("--llvm");
        auto should_bacteria = test_bacteria || test_llvm;
        auto should_parse = test_parser || should_bacteria;
        if (!(test_lexer || should_parse)) {
            std::cerr << "error: in test mode, at least one of '--lex', '--parse', '--bacteria', '--llvm' should be selected\n";
            std::cerr << program;
            std::exit(1);
        }
        auto files = program.get<std::vector<std::string>>("...");
        if (files.size() == 0) {
            std::cerr << "error: no input files\n";
            std::cerr << program;
            std::exit(1);
        }
        for (std::string& str : files) {
            std::cout << "[" << str << "]\n";
            std::ifstream inf(str);
            std::stringstream buffer;
            buffer << inf.rdbuf();
            std::string s = buffer.str();
            auto lexed = cheese::lexer::lex(std::string_view(s),str);
            if (test_lexer) {
                std::cout << "LEXED\n";
                cheese::lexer::output(lexed);
            }
            if (should_parse) {
                //Input parsing code here
                if (test_parser) {
                    std::cout << "PARSED\n";
                    std::cout << "todo: parsing\n";
                }
                if (should_bacteria) {
                    //Input bacteria code here
                    if (test_bacteria) {
                        std::cout << "BACTERIA\n";
                        std::cout << "todo: bacteria\n";
                    }
                    if (test_llvm) {
                        //Input LLVM code here
                        std::cout << "LLVM\n";
                        std::cout << "todo: LLVM";
                    }
                }
            }
        }
    }


    return 0;
}
