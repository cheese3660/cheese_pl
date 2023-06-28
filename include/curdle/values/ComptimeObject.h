//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMEOBJECT_H
#define CHEESE_COMPTIMEOBJECT_H

#include "curdle/comptime.h"

namespace cheese::curdle {
    struct ComptimeObject : ComptimeValue {
        void mark_value() override;

        std::unordered_map<std::string, ComptimeValue *> fields{};

        ComptimeObject(Type *ty) {
            type = ty;
        }

        ~ComptimeObject() override = default;

        bool is_same_as(ComptimeValue *other) override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        std::string to_string() override;
    };
}

#endif //CHEESE_COMPTIMEOBJECT_H
