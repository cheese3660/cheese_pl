//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMEFLOAT_H
#define CHEESE_COMPTIMEFLOAT_H

#include "curdle/comptime.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct ComptimeFloat : ComptimeValue {
        explicit ComptimeFloat(double value, Type *ty) : value(value) {
            type = ty;
        }

        void mark_value() override;

        double value;

        ~ComptimeFloat() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        gcref<ComptimeValue> op_unary_plus(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_unary_minus(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_multiply(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_divide(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_remainder(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_add(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_subtract(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_lesser_than(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_greater_than(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_lesser_than_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_greater_than_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_not_equal(GlobalContext *gctx, ComptimeValue *other) override;
    };
}
#endif //CHEESE_COMPTIMEFLOAT_H
