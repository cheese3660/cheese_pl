//
// Created by Lexi Allen on 10/2/2023.
//
#include "tools/build.h"
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
#include "bacteria/nodes/reciever_nodes.h"
#include "tools/tools.h"
#include "llvm/IR/LegacyPassManager.h"
#include <filesystem>
#include <llvm/Support/FileSystem.h>

namespace cheese::tools {
    namespace fs = std::filesystem;

    void output_module(std::string out, llvm::Module &mod, llvm::TargetMachine *machine) {
        llvm::legacy::PassManager pass;
        auto fileType = llvm::CodeGenFileType::CGFT_ObjectFile;
        std::error_code errorCode;
        llvm::raw_fd_ostream dest(out, errorCode, llvm::sys::fs::OF_None);
        if (errorCode) {
            llvm::errs() << "Could not open file: " << out << ": " << errorCode.message();
        }
        if (machine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
            llvm::errs() << "TargetMachine can't emit a file of this type";
        }
        pass.run(mod);
        dest.flush();
        dest.close();
    }

    int build(std::vector<std::string> args) {
        auto program = get_parser("translate");
        program.add_argument("--output", "-o").help(
                "The output file for the object").default_value(
                "translated.o").nargs(1);
        program.add_argument("--library", "-l").help(
                "The library folders to test translation from").default_value<std::vector<fs::path>>({}).append();
        program.add_argument("file").help("the file to parse");
        program.parse_args(args);
        process_common_arguments(program);
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
            auto project = cheese::project::Project{
                    fs::path{file}.parent_path(),
                    program.get<std::vector<fs::path>>("--library"),
                    fs::path{file},
                    parsed,
                    project::ProjectType::Application
            };
            auto machine = cheese::project::Machine{};
            auto gc = cheese::memory::garbage_collection::garbage_collector{64};
            auto ctx = gc.gcnew<cheese::project::GlobalContext>(project, gc, machine);
            auto node = curdle::curdle(ctx);
            std::cout << "Bacteria:\n";
            std::cout << node->get_textual_representation();
            auto prog = (bacteria::nodes::BacteriaProgram *) node.get();
            auto mod = prog->lower_into_module(ctx);
            std::cout << "LLVM:\n";
            mod->dump();
            output_module(out, *mod, machine.machine);
            return 0;
        } catch (std::exception &e) {
            std::cout << e.what() << '\n';
            return 1;
        }
    }
}