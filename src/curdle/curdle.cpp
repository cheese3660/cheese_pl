//
// Created by Lexi Allen on 3/20/2023.
//
#include <utility>

#include "curdle/curdle.h"

#include "curdle/GlobalContext.h"

#include "parser/parser.h"

#include "parser/nodes/OtherNodes.h"
#include "parser/nodes/SingleMemberNodes.h"
#include "parser/nodes/TerminalNodes.h"

using namespace cheese::memory::garbage_collection;

namespace cheese::curdle {

    // Likely going to need more parameters, but this should be fine
    gcref <Structure>
    translate_structure(GlobalContext &ctx, std::shared_ptr<parser::nodes::Structure> structure_node) {
        auto structure_ref = ctx.gc.gcnew<Structure>();

        if (structure_node->is_tuple) {

        } else {

        }

        return structure_ref;
    }

    void setup_root_structure(GlobalContext &ctx, parser::NodePtr ast_node) {
        garbage_collector &gc = ctx.gc;
        auto translated = translate_structure(ctx, std::dynamic_pointer_cast<parser::nodes::Structure>(ast_node));
        gc.add_root_object(translated.get());
        ctx.root_structure = translated.get();
    }


    bacteria::BacteriaPtr curdle(const Project &project) {
        // First off create a structure for the main project file
        GlobalContext ctx{project, nullptr, garbage_collector{64}}; // Lets do it every 64 iteratations
        setup_root_structure(ctx, project.root_file);
        return bacteria::BacteriaPtr{};
    }
}