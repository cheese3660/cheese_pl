//
// Created by Lexi Allen on 3/31/2023.
//
#include "curdle/comptime.h"
#include "project/GlobalContext.h"
#include "NotImplementedException.h"
#include "curdle/Type.h"
#include "typeinfo"
#include "stringutil.h"
#include <iostream>
#include <utility>
#include "curdle/builtin.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/values/ComptimeFloat.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/values/ComptimeType.h"
#include "curdle/values/ComptimeArray.h"
#include "curdle/values/ComptimeObject.h"
#include "curdle/values/ComptimeFunctionSet.h"
#include "curdle/values/BuiltinFunctionReference.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/VoidType.h"
#include "curdle/types/AnyType.h"
#include "curdle/types/Float64Type.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/values/ComptimeComplex.h"
#include "curdle/types/ComptimeComplexType.h"
#include "curdle/values/ComptimeEnumLiteral.h"
#include "curdle/types/ComptimeEnumType.h"
#include "curdle/types/ArrayType.h"
#include "curdle/types/PointerType.h"


namespace cheese::curdle {
    void ComptimeValue::mark_references() {
        type->mark();
        mark_value();
    }

#define OPERATOR_NOT_DEFINED_FOR(operator_name) throw CurdleError("Invalid Comptime Operation: " operator_name " cannot be used on a value of type " + type->to_string() + " at compile time", error::ErrorCode::NotComptime)

    gcref<ComptimeValue>
    ComptimeValue::op_tuple_call(cheese::project::GlobalContext *gctx, std::vector<ComptimeValue *> values) {
        OPERATOR_NOT_DEFINED_FOR("()");
    }

    gcref<ComptimeValue>
    ComptimeValue::op_array_call(cheese::project::GlobalContext *gctx, std::vector<ComptimeValue *> values) {
        OPERATOR_NOT_DEFINED_FOR("[]");
    }

    gcref<ComptimeValue>
    ComptimeValue::op_object_call(cheese::project::GlobalContext *gctx,
                                  std::unordered_map<std::string, ComptimeValue *> values) {
        OPERATOR_NOT_DEFINED_FOR("{}");
    }

    gcref<ComptimeValue> ComptimeValue::op_unary_plus(cheese::project::GlobalContext *gctx) {
        OPERATOR_NOT_DEFINED_FOR("unary+");
    }

    gcref<ComptimeValue> ComptimeValue::op_unary_minus(cheese::project::GlobalContext *gctx) {
        OPERATOR_NOT_DEFINED_FOR("unary-");
    }

    gcref<ComptimeValue> ComptimeValue::op_not(cheese::project::GlobalContext *gctx) {
        OPERATOR_NOT_DEFINED_FOR("not");
    }

    gcref<ComptimeValue> ComptimeValue::op_multiply(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("*");
    }

    gcref<ComptimeValue> ComptimeValue::op_divide(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("*");
    }

    gcref<ComptimeValue> ComptimeValue::op_remainder(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("%");
    }

    gcref<ComptimeValue> ComptimeValue::op_add(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("+");
    }

    gcref<ComptimeValue> ComptimeValue::op_subtract(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("-");
    }

    gcref<ComptimeValue> ComptimeValue::op_left_shift(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("<<");
    }

    gcref<ComptimeValue> ComptimeValue::op_right_shift(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR(">>");
    }

    gcref<ComptimeValue> ComptimeValue::op_lesser_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("<");
    }

    gcref<ComptimeValue> ComptimeValue::op_greater_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR(">");
    }

    gcref<ComptimeValue>
    ComptimeValue::op_lesser_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("<=");
    }

    gcref<ComptimeValue>
    ComptimeValue::op_greater_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR(">=");
    }

    gcref<ComptimeValue> ComptimeValue::op_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("==");
    }

    gcref<ComptimeValue> ComptimeValue::op_not_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("!=");
    }

    gcref<ComptimeValue> ComptimeValue::op_and(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("and");
    }

    gcref<ComptimeValue> ComptimeValue::op_xor(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("xor");
    }

    gcref<ComptimeValue> ComptimeValue::op_or(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("or");
    }

    gcref<ComptimeValue> ComptimeValue::op_combine(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        OPERATOR_NOT_DEFINED_FOR("&");
    }

    memory::garbage_collection::gcref<Type>
    ComptimeValue::binary_peer_lhs(Type *other_type, bool &cast_self, cheese::project::GlobalContext *gctx) {
        auto peer = peer_type({type, other_type}, gctx);
        auto lhs_compare = peer->compare(type);
        if (lhs_compare == -1)
            throw CurdleError(
                    "Bad Compile Time Cast: cannot cast a value of type " + type->to_string() + " to a value of type " +
                    peer->to_string(), error::ErrorCode::BadComptimeCast);
        cast_self = lhs_compare != 0;
        return peer;
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


    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(cheese::project::GlobalContext *gctx, T *ref) {
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

    gcref<ComptimeValue> ComptimeContext::exec_object_call(parser::nodes::ObjectCall *call, RuntimeContext *rtime) {
        auto function = exec(call->object.get(), rtime);
        std::unordered_map<std::string, ComptimeValue *> arguments;
        std::vector<gcref<ComptimeValue>> arg_refs;
        for (const auto &node: call->args) {
            auto actual_arg = (parser::nodes::FieldLiteral *) node.get();
            auto arg = exec(actual_arg->value, rtime);
            arguments[actual_arg->name] = arg.get();
            arg_refs.push_back(std::move(arg));
        }
        return function->op_object_call(globalContext, arguments);
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
            WHEN_NODE_IS(parser::nodes::AnyType, pAnyType) {
                return create_from_type(globalContext, AnyType::get(globalContext));
            }
            WHEN_NODE_IS(parser::nodes::Float64, pFloat64) {
                return create_from_type(globalContext, Float64Type::get(globalContext));
            }
            WHEN_NODE_IS(parser::nodes::Complex64, pComplex64) {
                return create_from_type(globalContext, Complex64Type::get(globalContext));
            }
            WHEN_NODE_IS(parser::nodes::Structure, pStructure) {
                return create_from_type(globalContext, translate_structure(this, pStructure).get());
            }
            WHEN_NODE_IS(parser::nodes::ArrayType, pArrayType) {
                bool constant = pArrayType->constant;
                auto subtype_value = exec(pArrayType->child.get(), rtime);
                if (auto child_type = dynamic_cast<ComptimeType *>(subtype_value.get()); child_type) {
                    gcref<Type> subtype = {gc, child_type->typeValue};
                    std::vector<std::size_t> current_dimensions;
                    for (std::ptrdiff_t i = pArrayType->dimensions.size() - 1; i >= 0; i--) {
                        if (auto as_unknown = dynamic_cast<parser::nodes::UnknownSize *>(pArrayType->dimensions[i].get()); as_unknown) {
                            if (!current_dimensions.empty()) {
                                subtype = gc.gcnew<ArrayType>(subtype.get(), current_dimensions, constant);
                                current_dimensions = {};
                            }
                            subtype = gc.gcnew<PointerType>(subtype.get(), constant);
                        } else {
                            auto result = exec(pArrayType->dimensions[i], rtime);
                            if (auto as_integer = dynamic_cast<ComptimeInteger *>(result.get()); as_integer) {
                                if (as_integer->value > 0) {
                                    current_dimensions.insert(current_dimensions.begin(), as_integer->value);
                                } else {
                                    throw LocalizedCurdleError{
                                            "Invalid dimension: expected a dimension size that resolved to a positive integer greater than zero",
                                            pArrayType->dimensions[i]->location,
                                            error::ErrorCode::InvalidDimension
                                    };
                                }
                            } else {
                                throw LocalizedCurdleError{
                                        "Invalid dimension: expected a dimension size that resolved to an integer",
                                        pArrayType->dimensions[i]->location,
                                        error::ErrorCode::InvalidDimension
                                };
                            }
                        }
                    }
                    if (!current_dimensions.empty()) {
                        subtype = gc.gcnew<ArrayType>(subtype.get(), current_dimensions, constant);
                    }

                    return create_from_type(globalContext, subtype.get());
                } else {
                    throw LocalizedCurdleError{
                            "Expected Type: Expected a value convertible to a type",
                            pArrayType->child->location,
                            error::ErrorCode::ExpectedType
                    };
                }
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
            WHEN_NODE_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
                return gc.gcnew<ComptimeInteger>(pIntegerLiteral->value, ComptimeIntegerType::get(globalContext));
            }
            WHEN_NODE_IS(parser::nodes::StringLiteral, pStringLiteral) {
                return gc.gcnew<ComptimeString>(pStringLiteral->str, ComptimeIntegerType::get(globalContext));
            }

            WHEN_NODE_IS(parser::nodes::FloatLiteral, pFloatLiteral) {
                return gc.gcnew<ComptimeFloat>(pFloatLiteral->value, ComptimeFloatType::get(globalContext));
            }

            WHEN_NODE_IS(parser::nodes::ImaginaryLiteral, pImaginaryLiteral) {
                return gc.gcnew<ComptimeComplex>(0, pImaginaryLiteral->value, ComptimeComplexType::get(globalContext));
            }
            WHEN_NODE_IS(parser::nodes::EnumLiteral, pEnumLiteral) {
                return gc.gcnew<ComptimeEnumLiteral>(pEnumLiteral->name, ComptimeEnumType::get(globalContext));
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
                    } else if (auto as_type = dynamic_cast<ComptimeType *>(lhs.get()); as_type) {
                        return as_type->typeValue->get_child_comptime(pValueReference->name, globalContext);
                    } else {
                        NOT_IMPL_FOR("Non-objects (" + typeid(*lhs.get()).name() + ")");
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
                    // :(named_block) {
                    //     let tmp = condition
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
            // Ah fun, tuple calling at compile time this is going to be fun
            WHEN_NODE_IS(parser::nodes::TupleCall, pTupleCall) {
                return exec_tuple_call(pTupleCall, rtime);
            }
            WHEN_NODE_IS(parser::nodes::ObjectCall, pObjectCall) {
                return exec_object_call(pObjectCall, rtime);
            }
            WHEN_NODE_IS(parser::nodes::Match, pMatch) {
                auto match_value = exec(pMatch->value, rtime);
                NOT_IMPL;
            }
            WHEN_NODE_IS(parser::nodes::ObjectLiteral, pObjectLiteral) {
                auto ty = gc.gcnew<Structure>(globalContext->verify_name("::lit"), this, gc);
                ty->implicit_type = true;
                auto val = gc.gcnew<ComptimeObject>(ty);
                for (auto &child: pObjectLiteral->children) {
                    auto as_field = dynamic_cast<parser::nodes::FieldLiteral *>(child.get());
                    auto v = exec(as_field->value, rtime);
                    val->fields[as_field->name] = v;
                    ty->fields.push_back(StructureField{
                            as_field->name,
                            v->type,
                            true
                    });
                }
                return val;
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

    void ComptimeValue::binary_peer_error(std::string msg, error::ErrorCode errorCode) {
        throw CurdleError{std::move(msg), errorCode};
    }
}