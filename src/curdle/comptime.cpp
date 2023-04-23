//
// Created by Lexi Allen on 3/31/2023.
//
#include "curdle/comptime.h"
#include "curdle/GlobalContext.h"
#include "NotImplementedException.h"
#include "curdle/Type.h"
#include "typeinfo"


namespace cheese::curdle {
    void ComptimeValue::mark_references() {
        type->mark();
        mark_value();
    }

    void ComptimeContext::mark_references() {
        if (currentStructure != nullptr) {
            currentStructure->mark();
        }
        if (parent != nullptr) {
            parent->mark(); //Most of the time this should exit early
        }
        for (auto &value: comptimeVariables) {
            value.second->mark();
        }
    }

    void ComptimeVariable::mark_references() {
        declaringType->mark();
        value->mark();
    }

    void ComptimeInteger::mark_value() {

    }

    bool ComptimeInteger::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_int = dynamic_cast<ComptimeInteger *>(other); as_int) {
            return value == as_int->value;
        } else {
            return false;
        }
    }


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

    void ComptimeComplex::mark_value() {

    }

    bool ComptimeComplex::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_complex = dynamic_cast<ComptimeComplex *>(other); as_complex) {
            return a == as_complex->a && b == as_complex->b;
        } else {
            return false;
        }
    }

    void ComptimeType::mark_value() {
        typeValue->mark();
    }

    ComptimeType::ComptimeType(garbage_collector &gc, Type *pType) {
        type = TypeType::get(gc);
        typeValue = pType;
    }

    bool ComptimeType::is_same_as(ComptimeValue *other) {
        if (auto as_type = dynamic_cast<ComptimeType *>(other); as_type) {
            return typeValue->compare(as_type->typeValue) == 0;
        } else {
            return false;
        }
    }

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
    }

    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(garbage_collector &gc, T *ref) {
        auto type = new ComptimeType{gc, static_cast<Type *>(ref)};
        return gc.manage<ComptimeValue>(type);
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::NodePtr node, RuntimeContext *rtime) {
#define WHEN_NODE_IS(type, name) if (auto name = dynamic_cast<type*>(ptr); name)
        auto ptr = node.get();
        auto &gc = globalContext->gc;
        WHEN_NODE_IS(parser::nodes::SignedIntType, pSignedIntType) {
            return create_from_type(gc, IntegerType::get(gc, true, pSignedIntType->size));
        }
        WHEN_NODE_IS(parser::nodes::UnsignedIntType, pUnsignedIntType) {
            return create_from_type(gc, IntegerType::get(gc, false, pUnsignedIntType->size));
        }
        WHEN_NODE_IS(parser::nodes::Void, pVoid) {
            return create_from_type(gc, VoidType::get(gc));
        }
        NOT_IMPL_FOR(typeid(*ptr).name());
#undef WHEN_NODE_IS
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::nodes::Comptime *ctime, RuntimeContext *rtime) {
        NOT_IMPL;
    }
}