//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeFloat.h"
#include "curdle/curdle.h"
#include "error.h"
#include "curdle/types/Float64Type.h"


namespace cheese::curdle {
    void ComptimeFloat::mark_value() {

    }

    bool ComptimeFloat::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_float = dynamic_cast<ComptimeFloat *>(other); as_float) {
            return value == as_float->value;
        } else {
            return false;
        }
    }

    std::string ComptimeFloat::to_string() {
        return std::to_string(value);
    }

    gcref<ComptimeValue> ComptimeFloat::cast(Type *target_type, garbage_collector &garbageCollector) {
        if (type->compare(target_type) == 0) return gcref{garbageCollector, this};
        if (auto as_f64 = dynamic_cast<Float64Type *>(target_type); as_f64) {
            return garbageCollector.gcnew<ComptimeFloat>(value, target_type);
        }
        throw CurdleError{"Cannot cast " + type->to_string() + " to " + target_type->to_string() + " at compile time",
                          error::ErrorCode::InvalidCast};
    }
}