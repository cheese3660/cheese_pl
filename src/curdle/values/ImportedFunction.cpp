//
// Created by Lexi Allen on 10/1/2023.
//
#include "curdle/values/ImportedFunction.h"

namespace cheese::curdle {

    void ImportedFunction::mark_value() {
    }

    bool ImportedFunction::is_same_as(ComptimeValue *other) {
        if (auto as_set = dynamic_cast<ImportedFunction *>(other); as_set) {
            return as_set->function == function;
        } else {
            return false;
        }
    }

    std::string ImportedFunction::to_string() {
        return type->to_string() + ":" + function;
    }

    ImportedFunction::ImportedFunction(std::string function, Type *prototype) : function(function) {
        type = prototype;
    }

    gcref<ComptimeValue> ImportedFunction::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }
}