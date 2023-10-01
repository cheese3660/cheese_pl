//
// Created by Lexi Allen on 9/30/2023.
//
#include <utility>

#include "curdle/values/ComptimeEnumLiteral.h"
#include "curdle/curdle.h"
#include "stringutil.h"

namespace cheese::curdle {
    void ComptimeEnumLiteral::mark_value() {
        std::for_each(tuple_fields.begin(), tuple_fields.end(), [](ComptimeValue *v) { v->mark(); });
        std::for_each(structured_fields.begin(), structured_fields.end(),
                      [](std::pair<std::string, ComptimeValue *> kv) { kv.second->mark(); });
    }

    bool ComptimeEnumLiteral::is_same_as(ComptimeValue *other) {
        if (other == this) return true;
        if (other->type->compare(type) != 0) return false;
        if (auto as_enum = dynamic_cast<ComptimeEnumLiteral *>(other); as_enum) {
            if (value != as_enum->value) return false;
            if (tuple_fields.size() != as_enum->tuple_fields.size()) return false;
            for (int i = 0; i < tuple_fields.size(); i++) {
                if (!tuple_fields[i]->is_same_as(as_enum->tuple_fields[i])) return false;
            }
            if (structured_fields.size() != as_enum->structured_fields.size()) return false;
            for (const auto &kv: structured_fields) {
                if (!as_enum->structured_fields.contains(kv.first)) return false;
                if (!kv.second->is_same_as(as_enum->structured_fields[kv.first])) return false;
            }
            return true;
        } else {
            return false;
        }
    }

    std::string ComptimeEnumLiteral::to_string() {
        std::stringstream ss;
        ss << "." << value;
        if (tuple_fields.empty() && structured_fields.empty()) return ss.str();
        if (structured_fields.empty()) {
            ss << "(";
            for (int i = 0; i < tuple_fields.size(); i++) {
                ss << tuple_fields[i];
                if (i != tuple_fields.size() - 1) {
                    ss << ",";
                }
            }
            ss << ")";
        } else {
            ss << "{";
            bool startWithComma = false;
            for (const auto &kv: structured_fields) {
                if (startWithComma) ss << ",";
                else startWithComma = true;
                ss << kv.first << ":" << kv.second->to_string();
            }
            ss << "}";
        }
        return ss.str();
    }

    gcref<ComptimeValue> ComptimeEnumLiteral::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }

    ComptimeEnumLiteral::ComptimeEnumLiteral(std::string value, Type *type) : value(std::move(value)) {
        this->type = type;
    }

    ComptimeEnumLiteral::ComptimeEnumLiteral(std::string value, std::vector<ComptimeValue *> tuple_fields, Type *type)
            : value(std::move(value)), tuple_fields(std::move(tuple_fields)) {

        this->type = type;
    }

    ComptimeEnumLiteral::ComptimeEnumLiteral(std::string value,
                                             std::unordered_map<std::string, ComptimeValue *> structured_fields,
                                             Type *type) : value(std::move(value)),
                                                           structured_fields(std::move(structured_fields)) {

        this->type = type;
    }

    gcref<ComptimeValue> ComptimeEnumLiteral::op_tuple_call(GlobalContext *gctx, std::vector<ComptimeValue *> values) {
        return gctx->gc.gcnew<ComptimeEnumLiteral>(value, values, type);
    }

    gcref<ComptimeValue>
    ComptimeEnumLiteral::op_object_call(GlobalContext *gctx, std::unordered_map<std::string, ComptimeValue *> values) {
        return gctx->gc.gcnew<ComptimeEnumLiteral>(value, values, type);
    }

}