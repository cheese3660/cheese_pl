//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeFunctionSet.h"
#include "curdle/types/FunctionTemplateType.h"
#include "GlobalContext.h"

namespace cheese::curdle {

    void ComptimeFunctionSet::mark_value() {
        set->mark();
    }

    bool ComptimeFunctionSet::is_same_as(ComptimeValue *other) {
        if (auto as_set = dynamic_cast<ComptimeFunctionSet *>(other); as_set) {
            return as_set->set == set;
        } else {
            return false;
        }
    }

    std::string ComptimeFunctionSet::to_string() {
        return std::to_string((size_t) set);
    }

    ComptimeFunctionSet::ComptimeFunctionSet(FunctionSet *set, GlobalContext *gctx) : set(set) {
        type = FunctionTemplateType::get(gctx);
    }

    gcref<ComptimeValue> ComptimeFunctionSet::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }
}