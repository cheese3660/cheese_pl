//
// Created by Lexi Allen on 9/19/2022.
//
#include "tools/tools.h"
#include "tools/test.h"
#include "tools/parse.h"
#include "configuration.h"
#include <iostream>
#include "lexer/lexer.h"
#include "NotImplementedException.h"
#include "vterm.h"
#include "argparse/argparse.hpp"
#include "configuration.h"
#include <fstream>

namespace cheese::tools {
    const char *version = "0.0.1";
    std::unordered_map<std::string, CheeseTool> tools{
            {"test", test},
            {"parse", parse},
    };

    argparse::ArgumentParser get_parser(std::string name) {
        auto parser = argparse::ArgumentParser(name, version);
        parser.add_argument("--no-vterm")
                .help("disables virtual terminal escapes for errors and the like")
                .default_value(false)
                .implicit_value(true)
                .nargs(0);
        return parser;
    }

    void process_common_arguments(argparse::ArgumentParser &parser) {
        cheese::configuration::use_escape_sequences = !parser.get<bool>("--no-vterm");
        if (configuration::use_escape_sequences) {
            configuration::setup_escape_sequences();
        }
    }
}