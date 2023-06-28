//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMEARRAY_H
#define CHEESE_COMPTIMEARRAY_H

#include "curdle/comptime.h"

namespace cheese::curdle {
    struct ComptimeArray : ComptimeValue {
        void mark_value() override;

        std::vector<ComptimeValue *> values;

        ~ComptimeArray() override = default;

        bool is_same_as(ComptimeValue *other) override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        explicit ComptimeArray(Type *type, std::vector<ComptimeValue *> values) : values(std::move(values)) {
            this->type = type;
        }

        std::string to_string() override;
    };
}
#endif //CHEESE_COMPTIMEARRAY_H
