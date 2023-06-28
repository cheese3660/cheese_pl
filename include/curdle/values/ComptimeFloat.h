//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMEFLOAT_H
#define CHEESE_COMPTIMEFLOAT_H

#include "curdle/comptime.h"

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
    };
}
#endif //CHEESE_COMPTIMEFLOAT_H
