//
// Created by Lexi Allen on 4/1/2023.
//
#include "curdle/GlobalContext.h"
#include <algorithm>
#include "curdle/curdle.h"
#include <fstream>
#include <sstream>
#include <string_view>

namespace cheese::curdle {

    void GlobalContext::mark_references() {
        if (entry_function != nullptr) {
            entry_function->mark();
        }
        if (root_structure != nullptr) {
            root_structure->mark();
        }
        for (auto &imp: imports) {
            imp.second->mark();
        }
    }

    Structure *GlobalContext::import_structure(Coordinate location, std::string path, fs::path dir, fs::path pdir) {
#ifdef WIN32
        std::replace(path.begin(), path.end(), '/', '\\');
#endif
        auto path_chs = path + ".chs";
        auto local_import = fs::absolute(dir / path_chs);
        if (imports.contains(local_import)) {
            return imports[local_import];
        }
        if (fs::exists(local_import)) {
            // Now we import this file :)
            std::ifstream t(local_import);
            std::stringstream buffer;
            buffer << t.rdbuf();
            t.close();
            std::string sv = buffer.str();
            auto lexed = lexer::lex(sv, local_import.string());
            auto parsed = parser::parse(lexed);
            auto ctx = gc.gcnew<ComptimeContext>(this, local_import, pdir);
            auto structure = translate_structure(ctx, dynamic_cast<parser::nodes::Structure *>(parsed.get()));
            imports[local_import] = structure;
            return structure;
        }

        auto local_library = fs::absolute(dir / path / "lib.chs");

        if (imports.contains(local_library)) {
            return imports[local_library];
        }

        if (fs::exists(local_library)) {
            std::ifstream t(local_library);
            std::stringstream buffer;
            buffer << t.rdbuf();
            t.close();
            std::string sv = buffer.str();
            auto lexed = lexer::lex(sv, local_library.string());
            auto parsed = parser::parse(lexed);
            auto ctx = gc.gcnew<ComptimeContext>(this, local_import, local_library.parent_path());
            auto structure = translate_structure(ctx, dynamic_cast<parser::nodes::Structure *>(parsed.get()));
            imports[local_import] = structure;
            return structure;
        }

        for (const auto &l: project.library_folders) {
            auto lib_import = fs::absolute(l / path_chs);
            if (imports.contains(lib_import)) {
                return imports[lib_import];
            }

            if (fs::exists(lib_import)) {
                // Now we import this file :)
                std::ifstream t(lib_import);
                std::stringstream buffer;
                buffer << t.rdbuf();
                t.close();
                std::string sv = buffer.str();
                auto lexed = lexer::lex(sv, lib_import.string());
                auto parsed = parser::parse(lexed);
                auto ctx = gc.gcnew<ComptimeContext>(this, lib_import, lib_import.parent_path());
                auto structure = translate_structure(ctx, dynamic_cast<parser::nodes::Structure *>(parsed.get()));
                imports[lib_import] = structure;
                return structure;
            }

            auto lib_library = fs::absolute(l / path / "lib.chs");
            if (fs::exists(lib_library)) {
                std::ifstream t(lib_library);
                std::stringstream buffer;
                buffer << t.rdbuf();
                t.close();
                std::string sv = buffer.str();
                auto lexed = lexer::lex(sv, lib_library.string());
                auto parsed = parser::parse(lexed);
                auto ctx = gc.gcnew<ComptimeContext>(this, local_import, lib_library.parent_path());
                auto structure = translate_structure(ctx, dynamic_cast<parser::nodes::Structure *>(parsed.get()));
                imports[local_import] = structure;
                return structure;
            }

        }

        error::raise_exiting_error("curdle", "Unresolved import: " + path, location,
                                   error::ErrorCode::UnresolvedImport);
    }
}