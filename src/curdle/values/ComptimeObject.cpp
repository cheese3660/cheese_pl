//
// Created by Lexi Allen on 6/28/2023.
//

#include "curdle/values/ComptimeObject.h"

namespace cheese::curdle {


    void ComptimeObject::mark_value() {
        for (auto value: fields) {
            value.second->mark();
        }
    }

    bool ComptimeObject::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_object = dynamic_cast<ComptimeObject *>(other); as_object) {
            if (as_object->fields.size() != fields.size()) return false;
            for (auto &kv: fields) {
                if (!as_object->fields.contains(kv.first)) return false;
                if (!as_object->fields[kv.first]->is_same_as(kv.second)) return false;
            }
        } else {
            return false;
        }
        return false;
    }

    gcref<ComptimeValue> ComptimeObject::cast(Type *target_type, garbage_collector &garbageCollector) {
        if (auto as_struct = dynamic_cast<Structure *>(target_type); as_struct) {
            auto new_obj = garbageCollector.gcnew<ComptimeObject>(target_type);
//            for (auto &[name, value]: fields) {
//                new_obj->fields[name] = value->cast(as_struct->field_type(name), garbageCollector);
//            }
            for (auto &field: as_struct->fields) {
                new_obj->fields[field.name] = fields[field.name]->cast(field.type, garbageCollector);
            }
            return new_obj;
        }
        NOT_IMPL_FOR(typeid(*target_type).name());
    }

    std::string ComptimeObject::to_string() {
        std::string result = type->to_string() + '{';
        int i = 0;
        for (auto &[key, value]: fields) {
            result += key + ":" + value->to_string();
            if (i != fields.size() - 1) {
                result += ',';
            }
            i += 1;
        }
        return result + '}';
    }
}