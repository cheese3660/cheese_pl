//
// Created by Lexi Allen on 4/1/2023.
//
#include "tools/translate.h"
#include "configuration.h"
#include <iostream>
#include "lexer/lexer.h"
#include <fstream>
#include <sstream>
#include <string_view>
#include "parser/parser.h"
#include "curdle/curdle.h"
#include "project/Project.h"
#include "project/Machine.h"
#include <filesystem>

namespace cheese::tools {
    namespace fs = std::filesystem;

    int translate(std::vector<std::string> args) {
        auto program = get_parser("translate");
        program.add_argument("--output", "-o").help(
                "The output file for the translation program in bacteria").default_value(
                "translated.bact").nargs(1);
        program.add_argument("--library", "-l").help(
                "The library folders to test translation from").default_value<std::vector<fs::path>>({}).append();
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
            auto project = curdle::Project{
                    fs::path{file}.parent_path(),
                    program.get<std::vector<fs::path>>("--library"),
                    fs::path{file},
                    parsed,
                    curdle::ProjectType::Application
            };
            auto machine = curdle::Machine{};
            auto node = curdle::curdle(project, machine);
            std::cout << node->get_textual_representation();
            return 0;
        } catch (std::exception &e) {
            std::cout << e.what() << '\n';
            return 1;
        }
    }
}