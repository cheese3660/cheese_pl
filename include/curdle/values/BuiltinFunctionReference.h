//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_BUILTINFUNCTIONREFERENCE_H
#define CHEESE_BUILTINFUNCTIONREFERENCE_H

#include "curdle/comptime.h"
#include "curdle/types/BuiltinReferenceType.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct BuiltinFunctionReference : ComptimeValue {
        std::string name;
        Builtin *builtin;

        explicit BuiltinFunctionReference(std::string name, Builtin *builtin, GlobalContext *gctx) : name(name),
                                                                                                     builtin(builtin) {
            type = BuiltinReferenceType::get(gctx);
        }

        void mark_value() override;

        bool is_same_as(ComptimeValue *other) override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        std::string to_string() override;

        ~BuiltinFunctionReference() override = default;

    };
}

#endif //CHEESE_BUILTINFUNCTIONREFERENCE_H
