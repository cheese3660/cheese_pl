//
// Created by Lexi Allen on 9/28/2023.
//

#ifndef CHEESE_COMPTIMEBOOL_H
#define CHEESE_COMPTIMEBOOL_H

#include "curdle/comptime.h"
#include "curdle/types/BooleanType.h"
#include "project/GlobalContext.h"

namespace cheese::curdle {
    struct ComptimeBool : ComptimeValue {
        bool value;


        explicit ComptimeBool(bool value, Type *type) : value(value) {
            this->type = type;
        }

        void mark_value() override;

        bool is_same_as(ComptimeValue *other) override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        std::string to_string() override;

        gcref<ComptimeValue> op_not(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_not_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_and(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_xor(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_or(GlobalContext *gctx, ComptimeValue *other) override;

    };
}
#endif //CHEESE_COMPTIMEBOOL_H
