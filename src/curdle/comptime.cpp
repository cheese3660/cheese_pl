//
// Created by Lexi Allen on 3/31/2023.
//
#include "curdle/comptime.h"
#include "curdle/GlobalContext.h"
#include "NotImplementedException.h"
#include "curdle/Type.h"
#include "typeinfo"
#include "stringutil.h"
#include <iostream>


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

    std::string ComptimeInteger::to_string() {
        return static_cast<std::string>(value);
    }

    ComptimeInteger::ComptimeInteger(const math::BigInteger &value, Type *ty) : value(value) {
        type = ty;
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

    std::string ComptimeString::to_string() {
        return '"' + stringutil::escape(value) + '"';
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

    std::string ComptimeVoid::to_string() {
        return "void";
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

    std::string ComptimeFloat::to_string() {
        return std::to_string(value);
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

    std::string ComptimeComplex::to_string() {
        return std::to_string(a) + "+" + std::to_string(b) + "i";
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

    std::string ComptimeType::to_string() {
        return typeValue->to_string();
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
        return false;
    }


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

    ComptimeFunctionSet::ComptimeFunctionSet(FunctionSet *set) : set(set) {}

    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(garbage_collector &gc, T *ref) {
        auto type = new ComptimeType{gc, static_cast<Type *>(ref)};
        return gc.manage<ComptimeValue>(type);
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::NodePtr node, RuntimeContext *rtime) {
        return exec(node.get(), rtime);
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::nodes::Comptime *ctime, RuntimeContext *rtime) {
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::Node *node, RuntimeContext *rtime) {
#define WHEN_NODE_IS(type, name) if (auto name = dynamic_cast<type*>(node); name)
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
        WHEN_NODE_IS(parser::nodes::ValueReference, pValueReference) {
            // Do the same as below but throw errors on an invalid value reference
            auto gotten = get(pValueReference->name);
            if (gotten.has_value()) {
                return std::move(gotten.value());
            } else {
                globalContext->raise("referencing a non-extant variable", node->location,
                                     error::ErrorCode::InvalidVariableReference);
            }
        }
        NOT_IMPL_FOR(typeid(*node).name());
#undef WHEN_NODE_IS
    }

    std::optional<gcref<ComptimeValue>> ComptimeContext::try_exec(parser::Node *node, RuntimeContext *rtime) {
#define WHEN_NODE_IS(type, name) if (auto name = dynamic_cast<type*>(node); name)
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
        WHEN_NODE_IS(parser::nodes::ValueReference, pValueReference) {
            return get(pValueReference->name);
        }
        WHEN_NODE_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
            return gc.gcnew<ComptimeInteger>(pIntegerLiteral->value, ComptimeIntegerType::get(gc));
        }
        WHEN_NODE_IS(parser::nodes::EqualTo, pEqualTo) {
            auto lhs = try_exec(pEqualTo->lhs.get(), rtime);
            if (!lhs.has_value()) return std::optional<gcref<ComptimeValue>>{};
            auto rhs = try_exec(pEqualTo->rhs.get(), rtime);
            if (!rhs.has_value()) return std::optional<gcref<ComptimeValue>>{};
            auto lhsv = std::move(lhs.value());
            auto rhsv = std::move(rhs.value());
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "Equality comparisons"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        WHEN_NODE_IS(parser::nodes::Subtraction, pSubtraction) {
            auto lhs = try_exec(pSubtraction->lhs.get(), rtime);
            if (!lhs.has_value()) return std::optional<gcref<ComptimeValue>>{};
            auto rhs = try_exec(pSubtraction->rhs.get(), rtime);
            if (!rhs.has_value()) return std::optional<gcref<ComptimeValue>>{};
            auto lhsv = std::move(lhs.value());
            auto rhsv = std::move(rhs.value());
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "Subtraction"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        NOT_IMPL_FOR(typeid(*node).name());
#undef WHEN_NODE_IS
    }

    std::optional<gcref<ComptimeValue>> ComptimeContext::get(const std::string &name) {
        if (comptimeVariables.contains(name)) {
            return gcref<ComptimeValue>(globalContext->gc, comptimeVariables[name]->value);
        }
        if (currentStructure) {
            currentStructure->resolve_by_name(name);
            if (currentStructure->comptime_variables.contains(name)) {
                return gcref<ComptimeValue>(globalContext->gc, currentStructure->comptime_variables[name].value);
            }
            if (currentStructure->function_sets.contains(name)) {
                // At some point we have to combine function sets...
                return globalContext->gc.gcnew<ComptimeFunctionSet>(currentStructure->function_sets[name]);
            }
        }
        if (parent) {
            return parent->get(name);
        }
        return {};
    }
}