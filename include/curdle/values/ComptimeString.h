//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMESTRING_H
#define CHEESE_COMPTIMESTRING_H

#include "curdle/comptime.h"

namespace cheese::curdle {
    struct ComptimeString : ComptimeValue {
        void mark_value() override;

        std::string value;

        ~ComptimeString() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;
    };
}

#endif //CHEESE_COMPTIMESTRING_H
