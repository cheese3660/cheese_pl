//
// Created by Lexi Allen on 6/28/2023.
//

#include <utility>

#include "curdle/values/ComptimeString.h"
#include "curdle/curdle.h"
#include "stringutil.h"

namespace cheese::curdle {
    void ComptimeString::mark_value() {

    }

    bool ComptimeString::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_string = dynamic_cast<ComptimeString *>(other); as_string) {
            return value == as_string->value;
        } else {
            return false;
        }
    }

    std::string ComptimeString::to_string() {
        return '"' + stringutil::escape(value) + '"';
    }

    gcref<ComptimeValue> ComptimeString::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }

    ComptimeString::ComptimeString(std::string value) : value(std::move(value)) {}
}