//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeArray.h"
#include "error.h"
#include "curdle/curdle.h"

namespace cheese::curdle {
    void ComptimeArray::mark_value() {
        for (auto value: values) {
            value->mark();
        }
    }

    bool ComptimeArray::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_array = dynamic_cast<ComptimeArray *>(other); as_array) {
            if (as_array->values.size() != values.size()) return false;
            for (int i = 0; i < values.size(); i++) {
                if (!values[i]->is_same_as(as_array->values[i])) return false;
            }
            return true;
        } else {
            return false;
        }
    }

    gcref<ComptimeValue> ComptimeArray::cast(Type *target_type, garbage_collector &garbageCollector) {
        if (auto cur_struct = dynamic_cast<Structure *>(type); cur_struct) {
            if (auto other_struct = dynamic_cast<Structure *>(target_type); other_struct) {
                if (cur_struct->is_tuple == other_struct->is_tuple) {
                    if (cur_struct->is_tuple) {
                        auto new_values = std::vector<ComptimeValue *>();
                        auto refs = std::vector<gcref<ComptimeValue>>();
                        for (int i = 0; i < values.size(); i++) {
                            auto value = values[i]->cast(other_struct->fields[i].type, garbageCollector);
                            new_values.push_back(value);
                            refs.push_back(std::move(value));
                        }
                        return garbageCollector.gcnew<ComptimeArray>(target_type, std::move(new_values));
                    } else {
                        NOT_IMPL_FOR("struct casting");
                    }
                } else {
                    throw CurdleError(
                            "Cannot cast " + type->to_string() + " to " + target_type->to_string() + " at compile time",
                            error::ErrorCode::InvalidCast);
                }
            } else {
                throw CurdleError(
                        "Cannot cast " + type->to_string() + " to " + target_type->to_string() + " at compile time",
                        error::ErrorCode::InvalidCast);
            }
        }
        NOT_IMPL;
    }

    std::string ComptimeArray::to_string() {
        std::string result = type->to_string();
        char end = ']';
        if (auto as_structure = dynamic_cast<Structure *>(type); as_structure) {
            result += '(';
            end = ')';
        } else {
            result += '[';
        }
        for (int i = 0; i < values.size(); i++) {
            result += values[i]->to_string();
            if (i != values.size() - 1) {
                result += ',';
            }
        }
        result += end;
        return result;
    }
}