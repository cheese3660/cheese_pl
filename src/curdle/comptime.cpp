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
#include "curdle/ComptimeInteger.h"


namespace cheese::curdle {
    void ComptimeValue::mark_references() {
        type->mark();
        mark_value();
    }

#define OPERATOR_NOT_DEFINED_FOR(operator_name) throw CurdleError("Invalid Comptime Operation: " operator_name " cannot be used on a value of type " + type->to_string() + " at compile time", error::ErrorCode::NotComptime)

    gcref<ComptimeValue> ComptimeValue::op_tuple_call(GlobalContext *gctx, std::vector<ComptimeValue *> values) {
        OPERATOR_NOT_DEFINED_FOR("()");
    }

    gcref<ComptimeValue> ComptimeValue::op_array_call(GlobalContext *gctx, std::vector<ComptimeValue *> values) {
        OPERATOR_NOT_DEFINED_FOR("[]");
    }

    gcref<ComptimeValue>
    ComptimeValue::op_object_call(GlobalContext *gctx, std::unordered_map<std::string, ComptimeValue *> values) {
        OPERATOR_NOT_DEFINED_FOR("{}");
    }

    gcref<ComptimeValue> ComptimeValue::op_unary_plus(GlobalContext *gctx) {
        OPERATOR_NOT_DEFINED_FOR("unary+");
    }

    gcref<ComptimeValue> ComptimeValue::op_unary_minus(GlobalContext *gctx) {
        OPERATOR_NOT_DEFINED_FOR("unary-");
    }

    gcref<ComptimeValue> ComptimeValue::op_not(GlobalContext *gctx) {
        OPERATOR_NOT_DEFINED_FOR("not");
    }

    gcref<ComptimeValue> ComptimeValue::op_multiply(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("*");
    }

    gcref<ComptimeValue> ComptimeValue::op_divide(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("*");
    }

    gcref<ComptimeValue> ComptimeValue::op_remainder(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("%");
    }

    gcref<ComptimeValue> ComptimeValue::op_add(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("+");
    }

    gcref<ComptimeValue> ComptimeValue::op_subtract(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("-");
    }

    gcref<ComptimeValue> ComptimeValue::op_left_shift(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("<<");
    }

    gcref<ComptimeValue> ComptimeValue::op_right_shift(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR(">>");
    }

    gcref<ComptimeValue> ComptimeValue::op_lesser_than(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("<");
    }

    gcref<ComptimeValue> ComptimeValue::op_greater_than(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR(">");
    }

    gcref<ComptimeValue> ComptimeValue::op_lesser_than_equal(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("<=");
    }

    gcref<ComptimeValue> ComptimeValue::op_greater_than_equal(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR(">=");
    }

    gcref<ComptimeValue> ComptimeValue::op_equal(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("==");
    }

    gcref<ComptimeValue> ComptimeValue::op_not_equal(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("!=");
    }

    gcref<ComptimeValue> ComptimeValue::op_and(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("and");
    }

    gcref<ComptimeValue> ComptimeValue::op_xor(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("xor");
    }

    gcref<ComptimeValue> ComptimeValue::op_or(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("or");
    }

    gcref<ComptimeValue> ComptimeValue::op_combine(GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("&");
    }

#undef OPERATOR_NOT_DEFINED_FOR

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
        if (type->compare(target_type) == 0) return gcref{garbageCollector, this};
        if (auto as_f64 = dynamic_cast<Float64Type *>(target_type); as_f64) {
            return garbageCollector.gcnew<ComptimeFloat>(value, target_type);
        }
        throw CurdleError{"Cannot cast " + type->to_string() + " to " + target_type->to_string() + " at compile time",
                          error::ErrorCode::InvalidCast};
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
//        WHEN_FUNCTION_IS(ComptimeFunctionSet, pComptimeFunctionSet) {

//        }
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
        std::vector<ComptimeValue *> arguments;
        // Store all the references so they will be deallocated at the *correct* time
        std::vector<gcref<ComptimeValue>> value_refs;
        for (const auto &node: call->args) {
            auto arg = exec(node.get(), rtime);
            arguments.push_back(arg);
            value_refs.push_back(std::move(arg));
        }
        return function->op_tuple_call(globalContext, arguments);
//        NOT_IMPL_FOR(typeid(*fptr).name());
#undef WHEN_FUNCTION_IS
    }

    gcref<ComptimeValue> ComptimeContext::exec(parser::Node *node, RuntimeContext *rtime) {
        try {
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
                    throw LocalizedCurdleError(
                            "Bad Builtin Call: Builtin does not exist: " + pBuiltinReference->builtin,
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

#define BIN_OP(name) return lhs->op_##name(globalContext,rhs)
            WHEN_NODE_IS(parser::nodes::Multiplication, pMultiplication) {
                auto lhs = exec(pMultiplication->lhs.get(), rtime);
                auto rhs = exec(pMultiplication->rhs.get(), rtime);
                BIN_OP(multiply);
            }
            WHEN_NODE_IS(parser::nodes::Modulus, pModulus) {
                auto lhs = exec(pModulus->lhs.get(), rtime);
                auto rhs = exec(pModulus->rhs.get(), rtime);
                BIN_OP(remainder);
            }
            WHEN_NODE_IS(parser::nodes::Division, pDivision) {
                auto lhs = exec(pDivision->lhs.get(), rtime);
                auto rhs = exec(pDivision->rhs.get(), rtime);
                BIN_OP(divide);
            }
            WHEN_NODE_IS(parser::nodes::Addition, pAddition) {
                auto lhs = exec(pAddition->lhs.get(), rtime);
                auto rhs = exec(pAddition->rhs.get(), rtime);
                BIN_OP(add);
            }
            WHEN_NODE_IS(parser::nodes::Subtraction, pSubtraction) {
                auto lhs = exec(pSubtraction->lhs.get(), rtime);
                auto rhs = exec(pSubtraction->rhs.get(), rtime);
                BIN_OP(subtract);
            }
            WHEN_NODE_IS(parser::nodes::LeftShift, pLeftShift) {
                auto lhs = exec(pLeftShift->lhs.get(), rtime);
                auto rhs = exec(pLeftShift->rhs.get(), rtime);
                BIN_OP(left_shift);
            }
            WHEN_NODE_IS(parser::nodes::RightShift, pRightShift) {
                auto lhs = exec(pRightShift->lhs.get(), rtime);
                auto rhs = exec(pRightShift->rhs.get(), rtime);
                BIN_OP(right_shift);
            }
            WHEN_NODE_IS(parser::nodes::LesserThan, pLesserThan) {
                auto lhs = exec(pLesserThan->lhs.get(), rtime);
                auto rhs = exec(pLesserThan->rhs.get(), rtime);
                BIN_OP(lesser_than);
            }
            WHEN_NODE_IS(parser::nodes::GreaterThan, pGreaterThan) {
                auto lhs = exec(pGreaterThan->lhs.get(), rtime);
                auto rhs = exec(pGreaterThan->rhs.get(), rtime);
                BIN_OP(greater_than);
            }
            WHEN_NODE_IS(parser::nodes::LesserEqual, pLesserEqual) {
                auto lhs = exec(pLesserEqual->lhs.get(), rtime);
                auto rhs = exec(pLesserEqual->rhs.get(), rtime);
                BIN_OP(lesser_than_equal);
            }
            WHEN_NODE_IS(parser::nodes::GreaterEqual, pGreaterEqual) {
                auto lhs = exec(pGreaterEqual->lhs.get(), rtime);
                auto rhs = exec(pGreaterEqual->rhs.get(), rtime);
                BIN_OP(greater_than_equal);
            }
            WHEN_NODE_IS(parser::nodes::EqualTo, pEqualTo) {
                auto lhs = exec(pEqualTo->lhs.get(), rtime);
                auto rhs = exec(pEqualTo->rhs.get(), rtime);
                BIN_OP(equal);
            }
            WHEN_NODE_IS(parser::nodes::NotEqualTo, pNotEqualTo) {
                auto lhs = exec(pNotEqualTo->lhs.get(), rtime);
                auto rhs = exec(pNotEqualTo->rhs.get(), rtime);
                BIN_OP(not_equal);
            }
            WHEN_NODE_IS(parser::nodes::And, pAnd) {
                auto lhs = exec(pAnd->lhs.get(), rtime);
                auto rhs = exec(pAnd->rhs.get(), rtime);
                BIN_OP(and);
            }
            WHEN_NODE_IS(parser::nodes::Xor, pXor) {
                auto lhs = exec(pXor->lhs.get(), rtime);
                auto rhs = exec(pXor->rhs.get(), rtime);
                BIN_OP(xor);
            }
            WHEN_NODE_IS(parser::nodes::Or, pOr) {
                auto lhs = exec(pOr->lhs.get(), rtime);
                auto rhs = exec(pOr->rhs.get(), rtime);
                BIN_OP(or);
            }
            WHEN_NODE_IS(parser::nodes::Combination, pCombination) {
                auto lhs = exec(pCombination->lhs.get(), rtime);
                auto rhs = exec(pCombination->rhs.get(), rtime);
                BIN_OP(combine);
            }
            WHEN_NODE_IS(parser::nodes::UnaryMinus, pUnaryMinus) {
                auto child = exec(pUnaryMinus->child.get(), rtime);
                return child->op_unary_minus(globalContext);
            }
            WHEN_NODE_IS(parser::nodes::UnaryPlus, pUnaryPlus) {
                auto child = exec(pUnaryPlus->child.get(), rtime);
                return child->op_unary_plus(globalContext);
            }
            WHEN_NODE_IS(parser::nodes::Not, pNot) {
                auto child = exec(pNot->child.get(), rtime);
                return child->op_not(globalContext);
            }
#undef BIN_OP
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
                                                       " does not contain a field by the name of: " +
                                                       pValueReference->name,
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
            WHEN_NODE_IS(parser::nodes::If, pIf) {
                if (pIf->unwrap.has_value()) {
                    // This is where we do a wierd translation for optionals, which are going to be a builtin type
                    // But essentially it becomes
                    // named_block: {
                    //     let tmp = condition;
                    //     <==(named_block) if tmp has value
                    //         inner_named_block: {
                    //             let unwrap = value of tmp
                    //             <==(inner_named_block) body
                    //         }
                    //     else
                    //        els
                    // }
                    // But for this we need to implement optionals :3
                    NOT_IMPL_FOR("If unwrapping");
                } else {
                    auto condition = exec(pIf->condition, rtime);
                    NOT_IMPL;
                }
            }
            WHEN_NODE_IS(parser::nodes::AddressOf, pAddressOf) {
                throw CurdleError("Not Compile Time: Can't take address at compile time",
                                  error::ErrorCode::NotComptime);
            }
            WHEN_NODE_IS(parser::nodes::TupleLiteral, pTupleLiteral) {
                // Here we should have an implied type specifier inside the structure object, as that makes conversion easier
                auto vec = std::vector<gcref<ComptimeValue>>();
                auto actual = std::vector<ComptimeValue *>();
                auto ty = gc.gcnew<Structure>(globalContext->verify_name("::lit"), this, gc);
                ty->is_tuple = true;
                ty->implicit_type = true;
                size_t i = 0;
                for (auto &child: pTupleLiteral->children) {
                    auto value = exec(child, rtime);
                    actual.push_back(value);
                    ty->fields.push_back(StructureField{
                            "_" + std::to_string(i),
                            value->type,
                            true
                    });
                    vec.push_back(std::move(value));
                    i++;
                }
                return gc.gcnew<ComptimeArray>(ty, std::move(actual));
            }
            WHEN_NODE_IS(parser::nodes::ObjectCall, pObjectCall) {
                throw CurdleError("TESTING",
                                  error::ErrorCode::NotComptime);
            }
            WHEN_NODE_IS(parser::nodes::Match, pMatch) {
                throw CurdleError("TESTING",
                                  error::ErrorCode::NotComptime);
            }
            WHEN_NODE_IS(parser::nodes::ObjectLiteral, pObjectLiteral) {
                throw CurdleError("TESTING",
                                  error::ErrorCode::NotComptime);
            }
            WHEN_NODE_IS(parser::nodes::Return, pReturn) {
                throw CurdleError("Not Compile Time: Can't return at compile time",
                                  error::ErrorCode::NotComptime);
            }
            NOT_IMPL_FOR(typeid(*node).name());
#undef WHEN_NODE_IS
        } catch (const CurdleError &e) {
            throw LocalizedCurdleError(e.what(), node->location, e.code);
        }
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