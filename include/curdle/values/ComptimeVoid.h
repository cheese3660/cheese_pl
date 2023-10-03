//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMEVOID_H
#define CHEESE_COMPTIMEVOID_H

#include "curdle/comptime.h"

namespace cheese::curdle {
    struct ComptimeVoid : ComptimeValue {
        void mark_value() override;

        ~ComptimeVoid() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        ComptimeVoid(Type *ty);
    };
}

#endif //CHEESE_COMPTIMEVOID_H
