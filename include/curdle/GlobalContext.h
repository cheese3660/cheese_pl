//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_GLOBALCONTEXT_H
#define CHEESE_GLOBALCONTEXT_H

#include <utility>

#include "Project.h"
#include "Structure.h"
#include "memory/garbage_collection.h"
#include "error.h"
#include "functions.h"
#include "bacteria/BacteriaNode.h"
#include "bacteria/BacteriaReciever.h"
#include "bacteria/nodes/reciever_nodes.h"

namespace cheese::curdle {
    struct GlobalContext : managed_object {

        GlobalContext(const Project &project, garbage_collector &collector) : project(project), root_structure(nullptr),
                                                                              gc(collector), entry_function(nullptr) {
            global_reciever = std::make_unique<bacteria::nodes::BacteriaProgram>(project.root_file->location);
        }

        bacteria::Reciever global_reciever;

        const Project &project;
        Structure *root_structure;
        FunctionTemplate *entry_function;
        garbage_collector &gc;
        bool errored{false};
        std::map<std::filesystem::path, Structure *> imports;

        inline void raise(std::string message, Coordinate location, error::ErrorCode code) {
            errored = true;
            error::raise_error("curdle", std::move(message), location, code);
        }

        void mark_references() override;

        Structure *import_structure(Coordinate location, std::string path, fs::path dir, fs::path pdir);

        ~GlobalContext() override = default;

    };
}
#endif //CHEESE_GLOBALCONTEXT_H
