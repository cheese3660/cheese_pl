//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeComplex.h"

namespace cheese::curdle {
    void ComptimeComplex::mark_value() {

    }

    bool ComptimeComplex::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_complex = dynamic_cast<ComptimeComplex *>(other); as_complex) {
            return a == as_complex->a && b == as_complex->b;
        } else {
            return false;
        }
    }

    std::string ComptimeComplex::to_string() {
        return std::to_string(a) + "+" + std::to_string(b) + "i";
    }

    gcref<ComptimeValue> ComptimeComplex::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }
}