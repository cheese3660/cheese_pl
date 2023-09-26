//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_GLOBALCONTEXT_H
#define CHEESE_GLOBALCONTEXT_H

#include <utility>

#include "Project.h"
#include "curdle/types/Structure.h"
#include "memory/garbage_collection.h"
#include "error.h"
#include "curdle/functions.h"
#include "bacteria/BacteriaNode.h"
#include "bacteria/BacteriaReciever.h"
#include "bacteria/nodes/reciever_nodes.h"
#include "Machine.h"
#include <set>

namespace cheese::project {
    using namespace cheese::curdle;

    struct GlobalContext : managed_object {

        GlobalContext(const Project &project, garbage_collector &collector, const Machine &machine)
                : project(project), root_structure(nullptr),
                  gc(collector), entry_function(nullptr), machine(machine), llvm_context(llvm::LLVMContext()) {
            global_reciever = std::make_unique<bacteria::nodes::BacteriaProgram>(project.root_file->location);
        }

        bacteria::Reciever global_reciever;

        const Project &project;
        const Machine &machine;
        llvm::LLVMContext llvm_context;
        Structure *root_structure;
        FunctionTemplate *entry_function;
        garbage_collector &gc;
        bool errored{false};
        std::unordered_map<std::filesystem::path, Structure *> imports;
        size_t anonymous_struct_offset{0};
        size_t anonymous_variable_offset{0};
        std::set<std::string> all_struct_names;

        std::string verify_name(std::string struct_name);

        inline void raise(std::string message, Coordinate location, error::ErrorCode code) {
            errored = true;
            error::raise_error("curdle", std::move(message), location, code);
        }

        void mark_references() override;

        Structure *import_structure(Coordinate location, std::string path, fs::path dir, fs::path pdir);

        ~GlobalContext() override = default;

        std::string get_anonymous_variable(const std::string &base) {
            return "__" + base + "__" + std::to_string(anonymous_variable_offset++);
        }

        // This is a lookup for cached objects
        std::unordered_map<std::string, managed_object *> cached_objects;

        bool try_get_cached_object(std::string key, managed_object *&out_value);

    };
}
#endif //CHEESE_GLOBALCONTEXT_H
