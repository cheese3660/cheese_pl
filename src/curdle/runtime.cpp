//
// Created by Lexi Allen on 4/6/2023.
//
#include "curdle/runtime.h"
#include "project/GlobalContext.h"
#include "curdle/curdle.h"
#include "curdle/types/Structure.h"
#include "curdle/values/ComptimeFunctionSet.h"
#include "curdle/values/ComptimeType.h"
#include "curdle/types/ReferenceType.h"
#include "curdle/types/BooleanType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/Float64Type.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/NoReturnType.h"
#include "curdle/types/TypeType.h"
#include "curdle/types/VoidType.h"
#include "curdle/types/ErrorType.h"
#include "curdle/values/BuiltinFunctionReference.h"
#include "curdle/builtin.h"


namespace cheese::curdle {
    void RuntimeContext::mark_references() {
        for (auto &var: variables) {
            var.second.type->mark();
        }
        comptime->mark();
        if (parent) parent->mark();
        if (structure) structure->mark();

    }

    std::optional<RuntimeVariableInfo> RuntimeContext::get(const std::string &name) {
        // This thing is only runtime, comptime is done in the stage before;
        if (variables.contains(name)) {
            return variables[name];
        }
        if (parent) {
            auto v = parent->get(name);
            if (v.has_value()) {
                return v.value();
            }
        }
        if (structure) {
            if (structure->top_level_variables.contains(name)) {
                auto &v = structure->top_level_variables[name];
                return RuntimeVariableInfo{v.constant, v.mangled_name, v.type};
            }
        }
        if (comptime) {
            auto v = comptime->get(name);
            if (v.has_value()) {
                return RuntimeVariableInfo{true, name, v.value()->type};
            }
        }
        return {};
    }

    // This gets a type in the purest way, without the tainting of "Local" context
    gcref<Type> RuntimeContext::get_type(parser::Node *node) {
        auto &gc = comptime->globalContext->gc;
        auto empty_ctx = gc.gcnew<LocalContext>(this);
        return empty_ctx->get_type(node);
    }

    void LocalContext::mark_references() {
        runtime->mark();
        if (expected_type) {
            expected_type->mark();
        }
    }

    gcref<Type> get_function_call_type(LocalContext *lctx, parser::nodes::TupleCall *call) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto empty_ctx = gc.gcnew<LocalContext>(rctx);;
        if (auto attempt = cctx->try_exec(call->object.get(), rctx); attempt.has_value()) {
            auto ctime = attempt->get();
            if (auto function_set = dynamic_cast<ComptimeFunctionSet *>(ctime); function_set) {
                auto arg_nodes = call->args;
                ConcreteFunction *function = get_function(rctx, cctx, empty_ctx, function_set->set, arg_nodes);
                return {gc, function->returnType};
            }
            if (auto builtin = dynamic_cast<BuiltinFunctionReference *>(ctime); builtin) {
                std::vector<parser::Node *> args;
                for (auto &arg: call->args) {
                    args.push_back(arg.get());
                }
                if (builtin->builtin->runtime) {
                    return builtin->builtin->type_of(call->location, lctx, args);
                } else {
                    return {gc, builtin->builtin->exec(call->location, args, cctx, rctx)->type};
                }
            }
            NOT_IMPL_FOR(typeid(*ctime).name());
        } else if (auto subscript = dynamic_cast<parser::nodes::Subscription *>(call->object.get()); subscript) {
            auto subscript_type = rctx->get_type(subscript->lhs.get()).get();
            auto function = dynamic_cast<parser::nodes::ValueReference *>(subscript->rhs.get());
            if (!function) {
                NOT_IMPL_FOR("Possible function pointers");
            }
            auto structure = dynamic_cast<Structure *>(subscript_type);
            if (!structure) {
                auto ref = dynamic_cast<ReferenceType *>(subscript_type);
                if (!(structure = dynamic_cast<Structure *>(ref->child))) {
                    NOT_IMPL_FOR("Non structure subscriptions");
                }
            }
            // We are going to have to check interfaces and mixins at some point which will be fun, also function pointers
            if (!structure->function_sets.contains(function->name));
            parser::NodeList arg_nodes{};
            arg_nodes.push_back(subscript->lhs);
            for (auto &arg: call->args) {
                arg_nodes.push_back(arg);
            }
            ConcreteFunction *function2 = get_function(rctx, cctx, empty_ctx, structure->function_sets[function->name],
                                                       arg_nodes);
            return {gc, function2->returnType};
        } else {
            auto obj = call->object.get();
            NOT_IMPL_FOR("non compile time deductible functions of type " + typeid(*obj).name());
        }
    }

    Type *get_object_call_type(LocalContext *lctx, parser::nodes::ObjectCall *call) {

        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto empty_ctx = gc.gcnew<LocalContext>(rctx);
        if (auto attempt = cctx->try_exec(call->object.get(), rctx); attempt.has_value()) {
            auto ctime = attempt->get();
#define WHEN_CTIME_IS(type, name) if (auto name = dynamic_cast<type*>(ctime); name)
            WHEN_CTIME_IS(ComptimeFunctionSet, pComptimeFunctionSet) {
                // Essentially this is a wrapper that creates an anonymous object that calls the function with it :3
                // That or it has to look at the function declaration to match variables with names in the second case, which is less fun
                // Wait this is how we can do variable argument functions, a second overload of tuple call that creates a tuple out of values but that sounds like *absolute* hell
                NOT_IMPL_FOR("Functions");
            }
            WHEN_CTIME_IS(ComptimeType, pComptimeType) {
                auto tvalue = pComptimeType->typeValue;
#define WHEN_TYPE_IS(type, name) if (auto name = dynamic_cast<type*>(tvalue); name)
                WHEN_TYPE_IS(Structure, pStructure) {
                    if (pStructure->function_sets.contains("{}")) {
                        //todo: check if it contains one w/o a self argument
                        NOT_IMPL_FOR("Constructors");
                    } else {
                        return pStructure;
                    }
                }
#undef WHEN_TYPE_IS
                NOT_IMPL_FOR(tvalue->to_string());
            }
#undef WHEN_CTIME_IS


            NOT_IMPL_FOR(ctime->type->to_string());
        } else {
            NOT_IMPL_FOR("non compile time deductible objects");
        }
    }

    gcref<Type> getBinaryType(LocalContext *lctx, parser::NodePtr lhs, parser::NodePtr rhs) {
        auto gctx = lctx->runtime->comptime->globalContext;
        auto &gc = gctx->gc;
        auto pair = lctx->get_binary_type(lhs.get(), rhs.get());
        auto lhs_ty = pair.first.get();
        auto rhs_ty = pair.second.get();
        // Now we have to check if a type is a trivial arithmetic type
        if (trivial_arithmetic_type(lhs_ty) && trivial_arithmetic_type(rhs_ty)) {
            return gcref<Type>(gc, peer_type({lhs_ty, rhs_ty}, gctx));
        } else {
            NOT_IMPL_FOR(lhs_ty->to_string() + " & " + rhs_ty->to_string());
        }
    }

    std::pair<gcref<Type>, std::string> get_capture_type(garbage_collector &gc, Type *t, parser::Node *capture) {
#define WHEN_CAPTURE_IS(type, name) if (auto name = dynamic_cast<type*>(capture); name)
        WHEN_CAPTURE_IS(parser::nodes::CopyCapture, pCopyCapture) {
            return {{gc, t}, pCopyCapture->name};
        }
        WHEN_CAPTURE_IS(parser::nodes::RefCapture, pRefCapture) {
            return {gc.gcnew<ReferenceType>(t, false), pRefCapture->name};
        }
        WHEN_CAPTURE_IS(parser::nodes::ConstRefCapture, pConstRefCapture) {
            return {gc.gcnew<ReferenceType>(t, true), pConstRefCapture->name};
        }
#undef WHEN_CAPTURE_IS
        NOT_IMPL_FOR(typeid(*capture).name());
    }

    void setup_match_context(RuntimeContext *rctx, Type *match_type, parser::Node *match) {
#define WHEN_MATCH_IS(type, name) if (auto name = dynamic_cast<type*>(match); name)
        WHEN_MATCH_IS(parser::nodes::MatchValue, pMatchValue) {
            return;
        }
        WHEN_MATCH_IS(parser::nodes::MatchAll, pMatchAll) {
            return;
        }

#undef WHEN_MATCH_IS
        NOT_IMPL_FOR(typeid(*match).name());
    }


    gcref<Type> get_arm_type(LocalContext *lctx, Type *value_type, parser::nodes::MatchArm *arm) {
        auto &gc = lctx->runtime->comptime->globalContext->gc;
        // Now we have to setup the new runtime context
        auto new_rctx = gc.gcnew<RuntimeContext>(lctx->runtime, lctx->runtime->comptime, lctx->runtime->structure);
        auto &arm_matches = arm->matches;
        if (arm->store) {
            auto info = get_capture_type(gc, value_type, arm->store.value().get());
            new_rctx->variables[info.second] = RuntimeVariableInfo{true, info.second, info.first.get()};
        }
        for (auto &match: arm_matches) {
            setup_match_context(new_rctx, value_type, match.get());
        }
        return lctx->get_type(arm->body.get());
    }


    gcref<Type> get_subscript_type(LocalContext *lctx, parser::nodes::Subscription *subscription) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto subscript_type = rctx->get_type(subscription->lhs.get());
        auto subscript_ptr = subscript_type.get();
        bool reference = false;
#define WHEN_SUBSCRIPT_IS(type, name) if (auto name = dynamic_cast<type*>(subscript_ptr); name != nullptr)
#define WHEN_KEY_IS(type, name) if (auto name = dynamic_cast<type*>(subscription->rhs.get()); name != nullptr)
        WHEN_SUBSCRIPT_IS(ReferenceType, pReferenceType) {
            reference = true;
            subscript_ptr = pReferenceType->child;
        }
        WHEN_SUBSCRIPT_IS(TypeType, pTypeType) {
            auto value = cctx->exec(subscription->lhs.get(), rctx);
            auto type = dynamic_cast<ComptimeType *>(value.get());
            if (auto struct_type = dynamic_cast<Structure *>(type->typeValue); struct_type) {
                WHEN_KEY_IS(parser::nodes::ValueReference, pValueReference) {
                    auto &name = pValueReference->name;
                    struct_type->resolve_by_name(name);
                    try {
                        auto child = struct_type->get_child_comptime(name, gctx);
                        return {gc, child->type};
                    } catch (const CurdleError &comptime_error) {
                        if (struct_type->top_level_variables.contains(name)) {
                            auto var = struct_type->top_level_variables[name];
                            // I *really* need to add access control with like a `can_access_private_variables_from(struct* other)` to a structure, but that's not necessary just yet
                            return {gc, var.type};
                        }
                    }
                } else {
                    throw LocalizedCurdleError{
                            "invalid key type " + std::string(typeid(*subscription->rhs.get()).name()) +
                            " for structure child reference", subscription->rhs->location,
                            error::ErrorCode::InvalidSubscript};
                }
            } else {
                NOT_IMPL_FOR(typeid(*type->typeValue).name());
            }
        }
        WHEN_SUBSCRIPT_IS(Structure, pStructure) {
            // This is the hellish part, innit?
            WHEN_KEY_IS(parser::nodes::ValueReference, pValueReference) {
                if (pStructure->is_tuple) {
                    throw LocalizedCurdleError{
                            "Attempting to use non integer index into a tuple structure: " + pStructure->to_string(),
                            subscription->rhs->location, error::ErrorCode::InvalidSubscript};
                } else {
                    for (int i = 0; i < pStructure->fields.size(); i++) {
                        auto &field = pStructure->fields[i];
                        if (field.name == pValueReference->name) {
                            // This is where we just return the operator to get a field
                            return {gc, field.type};
                        }
                    }
                }
            }
            WHEN_KEY_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
                if (pStructure->is_tuple) {
                    return {gc, pStructure->fields[pIntegerLiteral->value].type};
                } else {
                    throw LocalizedCurdleError{
                            "Attempting to use an integer index into a non tuple structure: " + pStructure->to_string(),
                            subscription->rhs->location, error::ErrorCode::InvalidSubscript};
                }

            }
        }
        NOT_IMPL_FOR(typeid(*subscript_ptr).name());
#undef WHEN_SUBSCRIPT_IS
#undef WHEN_KEY_IS
    }

    // Might want to make this return a gcref, as it might at some point create new types, but it shouldn't
    gcref<Type> LocalContext::get_type(parser::Node *node) {
        auto gctx = runtime->comptime->globalContext;
        auto &gc = gctx->gc;
        try {
            auto execed = runtime->comptime->try_exec(node, runtime);
            if (execed.has_value()) {
                auto etype = execed.value()->type;
                if (expected_type) {
                    return {gc, (expected_type->compare(etype) >= 0) ? expected_type : etype};
                } else {
                    return {gc, etype};
                }
            }
#define WHEN_NODE_IS(type, name) if (auto name = dynamic_cast<type*>(node); name)
            WHEN_NODE_IS(parser::nodes::EqualTo, pEqualTo) {
                auto pair = get_binary_type(pEqualTo->lhs.get(), pEqualTo->rhs.get());
                auto lhs_ty = pair.first.get();
                auto rhs_ty = pair.second.get();
                // Now we have to check if a type is a trivial arithmetic type
                if (trivial_arithmetic_type(lhs_ty) && trivial_arithmetic_type(rhs_ty)) {
                    // Now we do the boolean type here :3
                    return {gc, BooleanType::get(gctx)};
                } else {
                    NOT_IMPL_FOR(std::string{"equal to w/ "} + lhs_ty->to_string() + " & " + rhs_ty->to_string());
                }
            }
            WHEN_NODE_IS(parser::nodes::LesserThan, pLesserThan) {
                auto pair = get_binary_type(pLesserThan->lhs.get(), pLesserThan->rhs.get());
                auto lhs_ty = pair.first.get();
                auto rhs_ty = pair.second.get();
                // Now we have to check if a type is a trivial arithmetic type
                if (trivial_arithmetic_type(lhs_ty) && trivial_arithmetic_type(rhs_ty)) {
                    // Now we do the boolean type here :3
                    return {gc, BooleanType::get(gctx)};
                } else {
                    NOT_IMPL_FOR(std::string{"less than to w/ "} + lhs_ty->to_string() + " & " + rhs_ty->to_string());
                }
            }
            WHEN_NODE_IS(parser::nodes::ValueReference, pValueReference) {
                auto gotten = runtime->get(pValueReference->name);
                if (!gotten.has_value()) {
                    throw LocalizedCurdleError(
                            "Invalid Variable Reference: " + pValueReference->name + " does not exist",
                            pValueReference->location, error::ErrorCode::InvalidVariableReference);
                }
                return {gc, runtime->get(pValueReference->name)->type};
            }
            WHEN_NODE_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
                if (expected_type && dynamic_cast<IntegerType *>(expected_type)) {
                    return {gc, expected_type};
                } else {
                    return {gc, ComptimeIntegerType::get(gctx)};
                }
            }
            WHEN_NODE_IS(parser::nodes::FloatLiteral, pFloatLiteral) {
                if (expected_type && (dynamic_cast<Float64Type *>(expected_type))) {
                    return {gc, expected_type};
                } else {
                    return {gc, ComptimeFloatType::get(gctx)};
                }
            }
            WHEN_NODE_IS(parser::nodes::Multiplication, pMultiplication) {
                return getBinaryType(this, pMultiplication->lhs, pMultiplication->rhs);
            }
            WHEN_NODE_IS(parser::nodes::Subtraction, pSubtraction) {
                return getBinaryType(this, pSubtraction->lhs, pSubtraction->rhs);
            }
            WHEN_NODE_IS(parser::nodes::Modulus, pModulus) {
                return getBinaryType(this, pModulus->lhs, pModulus->rhs);
            }
            WHEN_NODE_IS(parser::nodes::Division, pDivision) {
                return getBinaryType(this, pDivision->lhs, pDivision->rhs);
            }
            WHEN_NODE_IS(parser::nodes::Addition, pAddition) {
                return getBinaryType(this, pAddition->lhs, pAddition->rhs);
            }
            WHEN_NODE_IS(parser::nodes::And, pAnd) {
                return getBinaryType(this, pAnd->lhs, pAnd->rhs);
            }
            WHEN_NODE_IS(parser::nodes::Xor, pXor) {
                return getBinaryType(this, pXor->lhs, pXor->rhs);
            }
            WHEN_NODE_IS(parser::nodes::Or, pOr) {
                return getBinaryType(this, pOr->lhs, pOr->rhs);
            }
            WHEN_NODE_IS(parser::nodes::Not, pNot) {
                return get_type(pNot->child.get());
            }
            WHEN_NODE_IS(parser::nodes::UnaryMinus, pUnaryMinus) {
                return get_type(pUnaryMinus->child.get());
            }
            WHEN_NODE_IS(parser::nodes::UnaryPlus, pUnaryPlus) {
                return get_type(pUnaryPlus->child.get());
            }
            WHEN_NODE_IS(parser::nodes::TupleCall, pTupleCall) {
                // Now time to do a bunch of work to get the type of *one* function call
                return get_function_call_type(this, pTupleCall);
            }
            WHEN_NODE_IS(parser::nodes::Cast, pCast) {
                try {
                    auto ctimeValue = runtime->comptime->exec(pCast->rhs, runtime);
                    if (auto as_type = dynamic_cast<ComptimeType *>(ctimeValue.get()); as_type) {
                        return {gc, as_type->typeValue};
                    } else {
                        throw InvalidCastError("Must cast to a type");
                    }
                } catch (const NotComptimeError &) {
                    throw InvalidCastError("Must cast to a comptime known type");
                }
            }
            WHEN_NODE_IS(parser::nodes::Return, pReturn) {
                return {gc, NoReturnType::get(gctx)};
            }
            WHEN_NODE_IS(parser::nodes::ObjectCall, pObjectCall) {
                // Now time to do a bunch of work to get the type of *one* function call
                return {gc, get_object_call_type(this, pObjectCall)};
            }
            WHEN_NODE_IS(parser::nodes::Structure, pStructure) {
                return {gc, TypeType::get(gctx)};
            }
            WHEN_NODE_IS(parser::nodes::Subscription, pSubscription) {
                return get_subscript_type(this, pSubscription);
            }
            WHEN_NODE_IS(parser::nodes::Self, pSelf) {
                return {gc, runtime->get("self")->type};
            }
            WHEN_NODE_IS(parser::nodes::Destructure, pDestructure) {
                return {gc, VoidType::get(gctx)};
            }
            WHEN_NODE_IS(parser::nodes::ObjectLiteral, pObjectLiteral) {
                if (expected_type != nullptr) {
                    return {gc, expected_type};
                } else {
                    auto structure = gc.gcnew<Structure>(runtime->comptime->globalContext->verify_name("::anon"),
                                                         runtime->comptime, gc);
                    structure->implicit_type = true;
                    for (auto &value: pObjectLiteral->children) {
                        auto lit = (parser::nodes::FieldLiteral *) (value.get());
                        structure->fields.push_back({lit->name, runtime->get_type(lit), true});
                    }
                    return structure;
                }
            }
            WHEN_NODE_IS(parser::nodes::TupleLiteral, pTupleLiteral) {
                if (expected_type != nullptr) {
                    return {gc, expected_type};
                } else {
                    auto structure = gc.gcnew<Structure>(runtime->comptime->globalContext->verify_name("::anon_tuple"),
                                                         runtime->comptime, gc);
                    structure->is_tuple = true;
                    structure->implicit_type = true;
                    int i = 0;
                    for (auto &value: pTupleLiteral->children) {
                        structure->fields.push_back({"_" + std::to_string(i++), runtime->get_type(value.get()), true});
                    }
                    return structure;
                }
            }
            WHEN_NODE_IS(parser::nodes::Match, pMatch) {
                // We peer type all the arms together
                std::vector<gcref<Type>> all_referenced_types;
                std::vector<Type *> all_arm_types;
                auto value_type = runtime->get_type(pMatch->value.get());
                for (auto &arm: pMatch->arms) {
                    // Actually shit for each arm we have to set up our own context for each arm and such as they can have names that may have values, and that depends on the type of the object being matched
                    // So this is gonna be a fun problem
                    auto ty = get_arm_type(this, value_type, (parser::nodes::MatchArm *) arm.get());
                    all_arm_types.push_back(ty.get());
                    all_referenced_types.push_back(std::move(ty));
                }
                return {gc, peer_type(all_arm_types, gctx)};
            }
            WHEN_NODE_IS(parser::nodes::AddressOf, pAddressOf) {
                // Now we have to do a "get l-value type" function
                auto lvalue_type = runtime->get_lvalue_type(pAddressOf->child.get());
                // We have to ma
                return gc.gcnew<ReferenceType>(lvalue_type.first, lvalue_type.second);
            }
            NOT_IMPL_FOR(typeid(*node).name());
#undef WHEN_NODE_IS
        } catch (const CurdleError &e) {
            return {gc, ErrorType::get(gctx)};
        } catch (const NotImplementedException &notImplementedException) {
            throw LocalizedCurdleError(notImplementedException.what(), node->location,
                                       error::ErrorCode::GeneralCompilerError);
        }
    }


    std::pair<gcref<Type>, bool> RuntimeContext::get_lvalue_type(parser::Node *node) {
        auto gctx = comptime->globalContext;
        auto &gc = gctx->gc;
        try {
#define WHEN_NODE_IS(type, name) if (auto name = dynamic_cast<type*>(node); name)
            WHEN_NODE_IS(parser::nodes::ValueReference, pValueReference) {
                auto val = get(pValueReference->name);
                if (!val.has_value()) {
                    throw LocalizedCurdleError(
                            "Invalid Variable Reference: " + pValueReference->name + " does not exist",
                            pValueReference->location, error::ErrorCode::InvalidVariableReference);
                }
                return {{gc, val.value().type}, val.value().constant};
            }
            NOT_IMPL_FOR(typeid(*node).name());
#undef  WHEN_NODE_IS
        } catch (const NotImplementedException &notImplementedException) {
            throw LocalizedCurdleError(notImplementedException.what(), node->location,
                                       error::ErrorCode::GeneralCompilerError);
        }
    }

    std::pair<gcref<Type>, gcref<Type>> LocalContext::get_binary_type(parser::Node *lhs, parser::Node *rhs) {
        auto gctx = runtime->comptime->globalContext;
        auto &gc = gctx->gc;
        auto lhs_ty = get_type(lhs);
        // To make this easier
        auto sub_ctx = gc.gcnew<LocalContext>(this, lhs_ty);
        auto rhs_ty = sub_ctx->get_type(rhs);
        try {
            auto peer = peer_type({lhs_ty, rhs_ty}, gctx);
            auto peer_ctx = gc.gcnew<LocalContext>(this, peer);
            lhs_ty = peer_ctx->get_type(lhs);
            rhs_ty = peer_ctx->get_type(rhs);
        } catch (const NotImplementedException &ne) {
            throw;
        } catch (const std::exception &e) {

        }
        return {{gc, lhs_ty},
                {gc, rhs_ty}};
    }
}