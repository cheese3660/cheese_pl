//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeType.h"
#include "curdle/types/TypeType.h"

namespace cheese::curdle {
    void ComptimeType::mark_value() {
        typeValue->mark();
    }

    ComptimeType::ComptimeType(GlobalContext *gctx, Type *pType) {
        type = TypeType::get(gctx);
        typeValue = pType;
    }

    bool ComptimeType::is_same_as(ComptimeValue *other) {
        if (auto as_type = dynamic_cast<ComptimeType *>(other); as_type) {
            return typeValue->compare(as_type->typeValue) == 0;
        } else {
            return false;
        }
    }

    std::string ComptimeType::to_string() {
        return typeValue->to_string();
    }

    gcref<ComptimeValue> ComptimeType::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }
}