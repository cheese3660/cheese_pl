#include <iostream>
#include "lexer/lexer.h"
#include "NotImplementedException.h"
#include "vterm.h"
using namespace cheese::io;
#include <argparse/argparse.hpp>
#include "configuration.h"
const char* name = "cheese";
const char* version = "0.0.1";
const char* copyright = "(C) Lexi Allen";
int main(int argc, const char** argv) {
    argparse::ArgumentParser program(name,version);

    program.add_argument("mode")
        .help("the mode of the program, currently only debug is supported")
        .default_value<std::string>("version")
        .required();
    program.add_argument("--no-vterm")
        .help("disables virtual terminal escapes for errors and the like")
        .default_value(true)
        .implicit_value(false);

    return 0;
}
