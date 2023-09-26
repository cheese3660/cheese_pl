//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_CURDLE_H
#define CHEESE_CURDLE_H

#include "bacteria/BacteriaNode.h"
#include "project/Project.h"
#include "parser/nodes/other_nodes.h"
#include "parser/nodes/single_member_nodes.h"
#include "parser/nodes/terminal_nodes.h"
#include "curdle/types/Structure.h"
#include "curdle/comptime.h"
#include "curdle/runtime.h"
#include "error.h"
#include "project/Machine.h"
#include "project/GlobalContext.h"

using namespace cheese::project;
// This

namespace cheese::curdle {
    std::unique_ptr<bacteria::BacteriaNode> curdle(gcref<cheese::project::GlobalContext> gctx);

    gcref<Structure>
    translate_structure(ComptimeContext *ctx, parser::nodes::Structure *structure_node);


    void translate_function_body(RuntimeContext *rctx, parser::NodePtr functionBody);

    // TODO: Completely refactor all error generation systems with this and only raise errors where they are necessary
    class CurdleError : public std::runtime_error {

    public:
        CurdleError(std::string message, error::ErrorCode errorCode) : std::runtime_error(message), code(errorCode) {}

        error::ErrorCode code;
    };

    class LocalizedCurdleError : public std::runtime_error {

    public:
        LocalizedCurdleError(std::string message, Coordinate location, error::ErrorCode errorCode) : std::runtime_error(
                message), code(errorCode),
                                                                                                     location(
                                                                                                             location) {}

        error::ErrorCode code;
        Coordinate location;
    };
}
#endif //CHEESE_CURDLE_H
