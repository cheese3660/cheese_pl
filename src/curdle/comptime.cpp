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
#include "curdle/builtin.h"
#include "curdle/curdle.h"


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

    static gcref<ComptimeValue> new_value(garbage_collector &garbageCollector, ComptimeValue *value) {
        return garbageCollector.manage(value);
    }


    gcref<ComptimeValue> ComptimeInteger::cast(Type *target_type, garbage_collector &garbageCollector) {
#define WHEN_TYPE_IS(type, name) if (auto name = dynamic_cast<type*>(target_type); name)
        WHEN_TYPE_IS(ComptimeIntegerType, pComptimeIntegerType) {
            return new_value(garbageCollector, new ComptimeInteger(value, pComptimeIntegerType));
        }
        WHEN_TYPE_IS(IntegerType, pIntegerType) {
            // Now we must check if the value fits in that range
//            auto max = math::BigInteger(1,pIntegerType->size);
            math::BigInteger max, min;
            if (pIntegerType->sign) {
                max = math::BigInteger(1, pIntegerType->size - 1) - 1;
                min = -math::BigInteger(1, pIntegerType->size - 1);
            } else {
                max = math::BigInteger(1, pIntegerType->size) - 1;
                min = 0;
            }
            if (value < min || value > max) {
                throw InvalidCastError(
                        "Invalid Cast: cannot convert compile time known integer with value: " +
                        static_cast<std::string>(value) +
                        " to type: " + pIntegerType->to_string() + " as it cannot fit w/in range");
            }
            return new_value(garbageCollector, new ComptimeInteger(value, pIntegerType));
        }
        throw BadComptimeCastError(
                "Bad Compile Time Cast: Cannot convert value of type: " + type->to_string() + " to type: " +
                target_type->to_string());
#undef WHEN_TYPE_IS
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

    gcref<ComptimeValue> ComptimeString::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
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

    gcref<ComptimeValue> ComptimeVoid::cast(Type *target_type, garbage_collector &garbageCollector) {
        throw InvalidCastError("Invalid cast: cannot cast value of type void");
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

    gcref<ComptimeValue> ComptimeFloat::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
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

    gcref<ComptimeValue> ComptimeComplex::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }

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
        NOT_IMPL;
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

    gcref<ComptimeValue> ComptimeObject::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
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

    ComptimeFunctionSet::ComptimeFunctionSet(FunctionSet *set, GlobalContext *gctx) : set(set) {
        type = FunctionTemplateType::get(gctx);
    }

    gcref<ComptimeValue> ComptimeFunctionSet::cast(Type *target_type, garbage_collector &garbageCollector) {
        NOT_IMPL;
    }

    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(GlobalContext *gctx, T *ref) {
        auto type = new ComptimeType{gctx, static_cast<Type *>(ref)};
        return {gctx->gc, type};
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::NodePtr node, RuntimeContext *rtime) {
        return exec(node.get(), rtime);
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::nodes::Comptime *ctime, RuntimeContext *rtime) {
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeContext::exec_tuple_call(parser::nodes::TupleCall *call, RuntimeContext *rtime) {
        auto function = exec(call->object.get(), rtime);
        auto fptr = function.get();
#define WHEN_FUNCTION_IS(type, name) if (auto name = dynamic_cast<type*>(fptr); name)
        WHEN_FUNCTION_IS(ComptimeFunctionSet, pComptimeFunctionSet) {
            std::vector<PassedFunctionArgument> arguments;
            // Store all the references so they will be deallocated at the *correct* time
            std::vector<gcref<ComptimeValue>> value_refs;
            for (const auto &node: call->args) {
                auto arg = exec(node.get(), rtime);
                arguments.push_back(
                        PassedFunctionArgument{false, arg, arg->type});
                value_refs.push_back(std::move(arg));
            }
            NOT_IMPL_FOR("Functions");
        }
        WHEN_FUNCTION_IS(BuiltinFunctionReference, pBuiltinFunctionReference) {
            if (pBuiltinFunctionReference->builtin->comptime) {
                std::vector<parser::Node *> arguments;
                for (auto &arg: call->args) {
                    arguments.push_back(arg.get());
                }
                return pBuiltinFunctionReference->builtin->exec(call->location, arguments, this, rtime);
            } else {
                throw LocalizedCurdleError(
                        "Bad Builtin Call: Attempting to execute runtime only builtin at comptime: " +
                        pBuiltinFunctionReference->name, call->location, error::ErrorCode::BadBuiltinCall);
            }
        }
        NOT_IMPL_FOR(typeid(*fptr).name());
#undef WHEN_FUNCTION_IS
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::Node *node, RuntimeContext *rtime) {
#define WHEN_NODE_IS(type, name) if (auto name = dynamic_cast<type*>(node); name)
        auto &gc = globalContext->gc;
        WHEN_NODE_IS(parser::nodes::SignedIntType, pSignedIntType) {
            return create_from_type(globalContext, IntegerType::get(globalContext, true, pSignedIntType->size));
        }
        WHEN_NODE_IS(parser::nodes::UnsignedIntType, pUnsignedIntType) {
            return create_from_type(globalContext, IntegerType::get(globalContext, false, pUnsignedIntType->size));
        }
        WHEN_NODE_IS(parser::nodes::Void, pVoid) {
            return create_from_type(globalContext, VoidType::get(globalContext));
        }
        WHEN_NODE_IS(parser::nodes::ValueReference, pValueReference) {
            // Do the same as below but throw errors on an invalid value reference
            auto gotten = get(pValueReference->name);
            if (gotten.has_value()) {
                return std::move(gotten.value());
            } else {
                throw LocalizedCurdleError(
                        "Compile Time Execution Error: referencing a non-extant compile time variable: " +
                        pValueReference->name, pValueReference->location, error::ErrorCode::NotComptime);
            }
        }
        WHEN_NODE_IS(parser::nodes::AnyType, pAnyType) {
            return create_from_type(globalContext, AnyType::get(globalContext));
        }
        // Ah fun, tuple calling at compile time this is going to be fun
        WHEN_NODE_IS(parser::nodes::TupleCall, pTupleCall) {
            return exec_tuple_call(pTupleCall, rtime);
        }
        WHEN_NODE_IS(parser::nodes::BuiltinReference, pBuiltinReference) {
            if (builtins.contains(pBuiltinReference->builtin)) {
                auto &builtin = builtins.at(pBuiltinReference->builtin);
                if (builtin.comptime || builtin.runtime) {
                    return gc.gcnew<BuiltinFunctionReference>(pBuiltinReference->builtin, &builtin, globalContext);
                } else {
                    return builtin.get(pBuiltinReference->location, this, rtime);
                }
            } else {
                throw LocalizedCurdleError("Bad Builtin Call: Builtin does not exist: " + pBuiltinReference->builtin,
                                           pBuiltinReference->location, error::ErrorCode::BadBuiltinCall);
            }
        }
        WHEN_NODE_IS(parser::nodes::Structure, pStructure) {
            return create_from_type(globalContext, translate_structure(this, pStructure).get());
        }
        WHEN_NODE_IS(parser::nodes::Float64, pFloat64) {
            return create_from_type(globalContext, Float64Type::get(globalContext));
        }
        WHEN_NODE_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
            return gc.gcnew<ComptimeInteger>(pIntegerLiteral->value, ComptimeIntegerType::get(globalContext));
        }

        WHEN_NODE_IS(parser::nodes::FloatLiteral, pFloatLiteral) {
            return gc.gcnew<ComptimeFloat>(pFloatLiteral->value, ComptimeFloatType::get(globalContext));
        }

        WHEN_NODE_IS(parser::nodes::EqualTo, pEqualTo) {
            auto lhs = exec(pEqualTo->lhs.get(), rtime);
            auto rhs = exec(pEqualTo->rhs.get(), rtime);
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "Equality comparisons"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        WHEN_NODE_IS(parser::nodes::LesserThan, pLesserThan) {
            auto lhs = exec(pLesserThan->lhs.get(), rtime);
            auto rhs = exec(pLesserThan->rhs.get(), rtime);
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "LesserThan"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        WHEN_NODE_IS(parser::nodes::Subtraction, pSubtraction) {
            auto lhs = exec(pSubtraction->lhs.get(), rtime);
            auto rhs = exec(pSubtraction->rhs.get(), rtime);
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "LesserThan"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        WHEN_NODE_IS(parser::nodes::Multiplication, pMultiplication) {
            auto lhs = exec(pMultiplication->lhs.get(), rtime);
            auto rhs = exec(pMultiplication->rhs.get(), rtime);
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "LesserThan"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        WHEN_NODE_IS(parser::nodes::Modulus, pModulus) {
            auto lhs = exec(pModulus->lhs.get(), rtime);
            auto rhs = exec(pModulus->rhs.get(), rtime);
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "LesserThan"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        WHEN_NODE_IS(parser::nodes::Division, pDivision) {
            auto lhs = exec(pDivision->lhs.get(), rtime);
            auto rhs = exec(pDivision->rhs.get(), rtime);
            // Now we must define a few functions
            NOT_IMPL_FOR(
                    "LesserThan"); // This will be "fun" to mandate all these functions in the comptime value structure
        }
        WHEN_NODE_IS(parser::nodes::Cast, pCast) {
            auto lhs = exec(pCast->lhs.get(), rtime);
            gcref<ComptimeValue> rhs = exec(pCast->rhs.get(), rtime);
            if (auto as_type = dynamic_cast<ComptimeType *>(rhs.get()); as_type) {
                return lhs->cast(as_type->typeValue, gc);
            } else {
                throw LocalizedCurdleError("Expected Type: Expected a value convertible to a type",
                                           pCast->rhs->location,
                                           error::ErrorCode::ExpectedType);
            }
        }
        WHEN_NODE_IS(parser::nodes::Subscription, pSubscription) {
            auto lhs = exec(pSubscription->lhs.get(), rtime);
            auto rhs_ptr = pSubscription->rhs.get();
#define WHEN_RHS_IS(type, name) if (auto name = dynamic_cast<type*>(rhs_ptr); name)
            WHEN_RHS_IS(parser::nodes::ValueReference, pValueReference) {
                if (auto as_object = dynamic_cast<ComptimeObject *>(lhs.get()); as_object) {
                    if (!as_object->fields.contains(pValueReference->name)) {
                        throw LocalizedCurdleError("Invalid Subscript: " + as_object->type->to_string() +
                                                   " does not contain a field by the name of: " + pValueReference->name,
                                                   pSubscription->location, error::ErrorCode::InvalidSubscript);
                    } else {
                        return {gc, as_object->fields[pValueReference->name]};
                    }
                } else {
                    NOT_IMPL_FOR("Non-objects");
                }
            }
            WHEN_RHS_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
                if (auto as_object = dynamic_cast<ComptimeObject *>(lhs.get()); as_object) {
                    auto field_name = static_cast<std::string>(pIntegerLiteral->value);
                    // We have to do a lot more once interfaces and such are a thing at compile time
                    if (!as_object->fields.contains(field_name)) {
                        throw LocalizedCurdleError("Invalid Subscript: " + as_object->type->to_string() +
                                                   " does not contain a field by the name of: " + field_name,
                                                   pSubscription->location, error::ErrorCode::InvalidSubscript);
                    } else {
                        return {gc, as_object->fields[field_name]};
                    }
                } else {
                    NOT_IMPL_FOR("Non-objects");
                }
            }
            NOT_IMPL_FOR("Non integer/name indices");
#undef WHEN_RHS_IS
        }
        WHEN_NODE_IS(parser::nodes::Self, pSelf) {
            auto gotten = get("self");
            if (gotten.has_value()) {
                return std::move(gotten.value());
            } else {
                throw LocalizedCurdleError(
                        "Compile Time Execution Error: referencing a non-extant compile time variable: self",
                        pSelf->location, error::ErrorCode::NotComptime);
            }
        }
        NOT_IMPL_FOR(typeid(*node).name());
#undef WHEN_NODE_IS
    }

//    std::optional<gcref<ComptimeValue>>
//    ComptimeContext::try_exec_tuple_call(parser::nodes::TupleCall *call, RuntimeContext *rtime) {
//        auto function = exec(call->object.get(), rtime);
//        auto fptr = function.get();
//#define WHEN_FUNCTION_IS(type, name) if (auto name = dynamic_cast<type*>(fptr); name)
//        WHEN_FUNCTION_IS(ComptimeFunctionSet, pComptimeFunctionSet) {
//            std::vector<PassedFunctionArgument> arguments;
//            // Store all the references so they will be deallocated at the *correct* time
//            std::vector<gcref<ComptimeValue>> value_refs;
//            for (const auto &node: call->args) {
//                auto arg = try_exec(node.get(), rtime);
//                if (!arg.has_value()) {
//                    return {};
//                }
//                arguments.push_back(
//                        PassedFunctionArgument{false, arg.value(), arg.value()->type});
//                value_refs.push_back(std::move(arg.value()));
//            }
//            NOT_IMPL_FOR("Functions");
//        }
//        WHEN_FUNCTION_IS(BuiltinFunctionReference, pBuiltinFunctionReference) {
//            if (pBuiltinFunctionReference->builtin->comptime) {
//                std::vector<parser::Node *> arguments;
//                for (auto &arg: call->args) {
//                    arguments.push_back(arg.get());
//                }
//                return pBuiltinFunctionReference->builtin->exec(call->location, arguments, this, rtime);
//            } else {
//                return {};
//            }
//        }
//        NOT_IMPL_FOR(typeid(*fptr).name());
//#undef WHEN_FUNCTION_IS
//    }

    std::optional<gcref<ComptimeValue>> ComptimeContext::try_exec(parser::Node *node, RuntimeContext *rtime) {
        try {
            return exec(node, rtime);
        } catch (const CurdleError &e) {
            return {};
        } catch (const LocalizedCurdleError &e) {
            return {};
        }
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
                auto function_set = currentStructure->function_sets[name];
                auto new_function_set = new ComptimeFunctionSet(function_set,
                                                                globalContext);
                return gcref<ComptimeValue>(globalContext->gc, new_function_set);
            }
        }
        if (parent) {
            return parent->get(name);
        }
        return {};
    }

    std::string ComptimeContext::get_structure_name() {
        if (structure_name_stack.empty()) {
            if (parent) {
                return parent->get_structure_name();
            } else {
                return globalContext->verify_name("");
            }
        } else {
            return globalContext->verify_name(structure_name_stack.back());
        }
    }

    void ComptimeContext::push_structure_name(std::string n) {
        structure_name_stack.push_back(n);
    }

    void ComptimeContext::pop_structure_name() {
        if (!structure_name_stack.empty()) {
            structure_name_stack.pop_back();
        }
    }


    BadComptimeCastError::BadComptimeCastError(const std::string &message) : runtime_error(message) {

    }
}