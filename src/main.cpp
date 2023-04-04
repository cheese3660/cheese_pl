#include <iostream>
#include "lexer/lexer.h"
#include "NotImplementedException.h"
#include "vterm.h"
#include "argparse/argparse.hpp"
#include "configuration.h"
#include <fstream>
#include "tools/tools.h"
#include <iostream>

std::string usage =
        "usage: cheese [tool] ...\n"
        "   cheese tool suite\n"
        "possible tools:\n"
        "   test        -   tests parts of the compiler\n"
        "   parse       -   parse a source file into an AST representation\n"
        "   translate   -   translate a program to bacteria\n"
        "options:\n"
        "   --version   -   print the version and exit\n"
        "   --help      -   print a help string dependant on the tool and exit\n";
std::string version = "cheese v0.0.1\n";


// Do all this because I need to use the external console and rider doesn't allow me to pause on exit
//#ifdef DEBUG
//
//void pause() {
//    std::cout << "Press enter to exit...";
//    std::cin.get();
//}
//
//#define RETURN(x) do { \
//    int code = x;   \
//    pause();\
//    return x;       \
//} while(0)
//#else
//#define RETURN(x) return x
//#endif
#define RETURN(x) return x

int main(int argc, const char **argv) {
    try {
        if (argc < 2) {
            std::cerr << "error: not enough arguments\n";
            std::cerr << usage;
            RETURN(1);
        }
        std::string mode{argv[1]};
        if (mode == "--version") {
            std::cout << version;
            RETURN(0);
        } else if (mode == "--help") {
            std::cout << usage;
            RETURN(1);
        } else {
            std::vector<std::string> args{mode};
            for (int i = 2; i < argc; i++) {
                args.push_back(argv[i]);
            }
            if (cheese::tools::tools.contains(mode)) {
                auto tool = cheese::tools::tools[mode];
                RETURN(tool(args));
            } else {
                std::cerr << "error: unknown tool\n";
                std::cerr << usage;
                RETURN(1);
            }
        }
    } catch (std::exception &e) {
        //If an exception makes it here, log it and exit
        std::cerr << "An exception has occurred: " << e.what() << "\n";
        return 1;
    }
}
