//
// Created by Lexi Allen on 9/19/2022.
//
#include "tools/test.h"
#include "configuration.h"
#include <iostream>
#include "lexer/lexer.h"
#include "NotImplementedException.h"
#include "vterm.h"
#include "argparse/argparse.hpp"
#include "configuration.h"
#include "tests/tests.h"
#include <fstream>
namespace cheese::tools {
    int test(std::vector<std::string> args) {
        auto program = get_parser("test");
        program.add_argument("--builtin", "-b")
                .help("run builtin tests ")
                .default_value(false)
                .implicit_value(true)
                .nargs(0);
        program.add_argument("...")
                .help("json based tests")
                .default_value<std::vector<std::string>>({})
                .append()
                .nargs(argparse::nargs_pattern::any);
        program.parse_args(args);
        auto should_test_builtin = program.get<bool>("--builtin");
        if (should_test_builtin) {
            tests::run_all_builtin();
        }
        auto files = program.get<std::vector<std::string>>("...");
        if (files.size() == 0 && !should_test_builtin) {
            std::cerr << "error: no tests to be run\n";
            std::cerr << program;
            std::exit(1);
        }
        for (std::string& testfile : files) {
            tests::run_json_tests(testfile);
        }
        return 0;
    }
}
