//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMEFUNCTIONSET_H
#define CHEESE_COMPTIMEFUNCTIONSET_H

#include "curdle/comptime.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct ComptimeFunctionSet : ComptimeValue {
        ComptimeFunctionSet(FunctionSet *set, GlobalContext *gc);

        FunctionSet *set;

        void mark_value() override;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        ~ComptimeFunctionSet() override = default;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

    };
}

#endif //CHEESE_COMPTIMEFUNCTIONSET_H
