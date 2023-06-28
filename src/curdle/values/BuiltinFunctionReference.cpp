//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/BuiltinFunctionReference.h"

namespace cheese::curdle {

    std::string BuiltinFunctionReference::to_string() {
        return "<builtin reference>";
    }

    gcref<ComptimeValue> BuiltinFunctionReference::cast(Type *target_type, garbage_collector &garbageCollector) {
        throw InvalidCastError("Invalid Cast: cannot cast a builtin type");
    }

    bool BuiltinFunctionReference::is_same_as(ComptimeValue *other) {
        return false;
    }

    void BuiltinFunctionReference::mark_value() {

    }
}