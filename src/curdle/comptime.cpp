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


    void ComptimeDouble::mark_value() {

    }

    void ComptimeString::mark_value() {

    }

    void ComptimeVoid::mark_value() {

    }

    void ComptimeFloat::mark_value() {

    }

    void ComptimeComplex32::mark_value() {

    }

    void ComptimeComplex64::mark_value() {

    }

    void ComptimeType::mark_value() {
        typeValue->mark();
    }

    ComptimeType::ComptimeType(garbage_collector &gc, Type *pType) {
        type = TypeType::get(gc);
        typeValue = pType;
    }

    void ComptimeArray::mark_value() {
        for (auto value: values) {
            value->mark();
        }
    }

    void ComptimeObject::mark_value() {
        for (auto value: fields) {
            value.second->mark();
        }
    }

    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(garbage_collector &gc, T *ref) {
        auto type = new ComptimeType{gc, static_cast<Type *>(ref)};
        return gc.manage<ComptimeValue>(type);
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::NodePtr node) {
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

    gcref<ComptimeValue> ComptimeContext::exec(parser::nodes::Comptime *ctime) {
        NOT_IMPL;
    }
}