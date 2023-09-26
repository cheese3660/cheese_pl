//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMECOMPLEX_H
#define CHEESE_COMPTIMECOMPLEX_H

#include "curdle/comptime.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct ComptimeComplex : ComptimeValue {
        void mark_value() override;

        double a;
        double b;

        ComptimeComplex(double a, double b, Type *type) : a(a), b(b) {
            this->type = type;
        }

        ~ComptimeComplex() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        gcref<ComptimeValue> op_unary_plus(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_unary_minus(GlobalContext *gctx) override;

        gcref<ComptimeValue> op_multiply(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_divide(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_add(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_subtract(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_equal(GlobalContext *gctx, ComptimeValue *other) override;

        gcref<ComptimeValue> op_not_equal(GlobalContext *gctx, ComptimeValue *other) override;
    };
}
#endif //CHEESE_COMPTIMECOMPLEX_H
