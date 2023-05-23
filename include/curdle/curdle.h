//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_CURDLE_H
#define CHEESE_CURDLE_H

#include "bacteria/BacteriaNode.h"
#include "curdle/Project.h"
#include "parser/nodes/other_nodes.h"
#include "parser/nodes/single_member_nodes.h"
#include "parser/nodes/terminal_nodes.h"
#include "curdle/Structure.h"
#include "curdle/comptime.h"
#include "curdle/runtime.h"
// This

namespace cheese::curdle {
    std::unique_ptr<bacteria::BacteriaNode> curdle(const Project &project);

    gcref<Structure>
    translate_structure(ComptimeContext *ctx, parser::nodes::Structure *structure_node);

    void translate_function_body(RuntimeContext *rctx, parser::NodePtr functionBody);
}
#endif //CHEESE_CURDLE_H
