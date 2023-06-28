//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeVoid.h"
#include "curdle/curdle.h"
#include "error.h"

namespace cheese::curdle {

    void ComptimeVoid::mark_value() {

    }

    bool ComptimeVoid::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_void = dynamic_cast<ComptimeVoid *>(other); as_void) {
            return true;
        } else {
            return false;
        }
    }

    std::string ComptimeVoid::to_string() {
        return "void";
    }

    gcref<ComptimeValue> ComptimeVoid::cast(Type *target_type, garbage_collector &garbageCollector) {
        throw InvalidCastError("Invalid cast: cannot cast value of type void");
    }
}