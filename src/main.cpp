#include <iostream>
#include "lexer/lexer.h"
#include "NotImplementedException.h"
#include "vterm.h"
#include "argparse/argparse.hpp"
#include "configuration.h"
#include <fstream>
#include "tools/tools.h"
std::string usage =
        "usage: cheese [tool] ...\n"
        "   cheese tool suite\n"
        "possible tools:\n"
        "   test        -   tests parts of the compiler\n"
        "options:\n"
        "   --version   -   print the version and exit\n"
        "   --help      -   print a help string dependant on the tool and exit\n";
std::string version = "cheese v0.0.1\n";
int main(int argc, const char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "error: not enough arguments\n";
            std::cerr << usage;
            return 1;
        }
        std::string mode{argv[1]};
        if (mode == "--version") {
            std::cout << version;
            return 0;
        } else if (mode == "--help") {
            std::cout << usage;
            return 1;
        } else {
            std::vector<std::string> args{mode};
            for (int i = 2; i < argc; i++) {
                args.push_back(argv[i]);
            }
            if (cheese::tools::tools.contains(mode)) {
                auto tool = cheese::tools::tools[mode];
                return tool(args);
            } else {
                std::cerr << "error: unknown tool\n";
                std::cerr << usage;
                return 1;
            }
        }
        return 0;
    } catch (std::exception& e) {
        //If an exception makes it here, log it and exit
        std::cerr << "An exception has occurred: " << e.what() << "\n";
        return 1;
    }
}
