//
// Created by Lexi Allen on 6/28/2023.
//

#include <utility>

#include "curdle/values/ComptimeString.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/types/ArrayType.h"
#include "curdle/types/PointerType.h"
#include "curdle/curdle.h"
#include "stringutil.h"
#include "curdle/types/IntegerType.h"

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
#define WHEN_TARGET_IS(type, name) if (auto name = dynamic_cast<type*>(target_type); name)
        WHEN_TARGET_IS(ComptimeStringType, pComptimeStringType) {
            return garbageCollector.gcnew<ComptimeString>(value, pComptimeStringType);
        }
        WHEN_TARGET_IS(ArrayType, pArrayType) {
            if (auto subtype = dynamic_cast<IntegerType *>(pArrayType->subtype);
                    subtype && subtype->size == 8 && !subtype->sign) {
                return garbageCollector.gcnew<ComptimeString>(value, pArrayType);
            }
        }
        WHEN_TARGET_IS(PointerType, pPointerType) {
            if (auto subtype = dynamic_cast<IntegerType *>(pPointerType->subtype);
                    subtype && subtype->size == 8 && !subtype->sign) {
                return garbageCollector.gcnew<ComptimeString>(value, pPointerType);
            }
        }
#undef WHEN_TARGET_IS
        throw CurdleError{
                "Bad compile time cast: cannot convert " + type->to_string() + " to " + target_type->to_string(),
                error::ErrorCode::BadComptimeCast
        };
    }

    ComptimeString::ComptimeString(std::string value, Type *type) : value(std::move(value)) {
        this->type = type;
    }
}