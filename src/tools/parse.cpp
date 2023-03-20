//
// Created by Lexi Allen on 3/20/2023.
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
#include <sstream>
#include <string_view>
#include "lexer/lexer.h"
#include "parser/parser.h"

namespace cheese::tools {

    int parse(std::vector<std::string> args) {
        auto program = get_parser("parse");
        program.add_argument("--output", "-o").help("The output file for the parsed program in JSON").default_value(
                "parsed.json").nargs(1);
        program.add_argument("file").help("the file to parse");
        program.parse_args(args);
        try {
            auto file = program.get("file");
            auto out = program.get("--output");
            configuration::die_on_first_error = false;
            std::ifstream t(file);
            std::stringstream buffer;
            buffer << t.rdbuf();
            t.close();
            std::string sv = buffer.str();
            auto lexed = lexer::lex(sv, file);
            auto parsed = parser::parse(lexed);
            auto json = parsed->as_json();
            auto dumped = json.dump(4, ' ');
            std::ofstream t2(out);
            t2 << dumped;
            t2.close();

        } catch (std::exception &e) {
            std::cout << e.what() << '\n';
            return 1;
        }
    }
}