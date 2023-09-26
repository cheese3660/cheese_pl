//
// Created by Lexi Allen on 5/28/2023.
//

#ifndef CHEESE_COMPTIMEINTEGER_H
#define CHEESE_COMPTIMEINTEGER_H

#include "curdle/comptime.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
// An integer of any integral type
    struct ComptimeInteger : ComptimeValue {
        explicit ComptimeInteger(const math::BigInteger &value, Type *ty);

        void mark_value() override;

        ~ComptimeInteger() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        gcref<ComptimeValue> op_unary_plus(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_unary_minus(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_not(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_multiply(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_divide(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_remainder(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_add(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_subtract(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_left_shift(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_right_shift(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_lesser_than(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_greater_than(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_lesser_than_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_greater_than_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_not_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_and(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_xor(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_or(GlobalContext *gctx, ComptimeValue *other) override;

        math::BigInteger value;
    };
}

#endif //CHEESE_COMPTIMEINTEGER_H
