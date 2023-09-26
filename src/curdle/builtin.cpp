//
// Created by Lexi Allen on 5/24/2023.
//

#include "curdle/builtin.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeType.h"
#include "project/GlobalContext.h"

namespace cheese::curdle {

    std::map<std::string, Builtin> builtins;

    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(cheese::project::GlobalContext *gctx, T *ref) {
        auto type = new ComptimeType{gctx, static_cast<Type *>(ref)};
        return {gctx->gc, type};
    }

    gcref<ComptimeValue>
    type_builtin(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                 RuntimeContext *rctx) {
        if (arguments.size() != 1) {
            throw LocalizedCurdleError(
                    "Attempting to use $Type w/ the wrong number of arguments, it only takes one argument", location,
                    error::ErrorCode::BadBuiltinCall);
        }
        if (!rctx) {
            return create_from_type(cctx->globalContext, cctx->exec(arguments[0], rctx)->type);
        } else {
            return create_from_type(cctx->globalContext, rctx->get_type(arguments[0]).get());
        }
    }

    BUILTIN("Type", type_builtin)

    gcref<ComptimeValue>
    peer_builtin(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                 RuntimeContext *rctx) {
        std::vector<Type *> all_types;
        std::vector<gcref<ComptimeValue>> local_references;
        for (auto argument: arguments) {
            auto type_value = cctx->exec(argument, rctx);
            if (auto correct = dynamic_cast<ComptimeType *>(type_value.get()); correct) {
                all_types.push_back(correct->typeValue);
            } else {
                throw LocalizedCurdleError(
                        "Attempting to use $Peer w/ an argument of type " + type_value->type->to_string(),
                        argument->location, error::ErrorCode::BadBuiltinCall);
            }
            local_references.push_back(std::move(type_value));
        }
        return create_from_type(cctx->globalContext, peer_type(all_types, cctx->globalContext).value);
    }

    BUILTIN("Peer", peer_builtin)

    BadBuiltinCall::BadBuiltinCall(const std::string &message) : runtime_error(message) {}
}