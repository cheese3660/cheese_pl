//
// Created by Lexi Allen on 4/6/2023.
//
#include "curdle/runtime.h"
#include "curdle/GlobalContext.h"


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
        return {};
    }

    // This gets a type in the purest way, without the tainting of "Local" context
    Type *RuntimeContext::get_type(parser::Node *node) {
        NOT_IMPL;
    }

    void LocalContext::mark_references() {
        runtime->mark();
        if (expected_type) {
            expected_type->mark();
        }
    }

    Type *get_function_call_type(LocalContext *lctx, parser::nodes::TupleCall *call) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto empty_ctx = gc.gcnew<LocalContext>(rctx);;
        if (auto attempt = cctx->try_exec(call->object.get(), rctx); attempt.has_value()) {
            auto ctime = attempt->get();
            if (auto function_set = dynamic_cast<ComptimeFunctionSet *>(ctime); function_set) {
                std::vector<PassedFunctionArgument> arguments;
                // Store all the references so they will be deallocated at the *correct* time
                std::vector<gcref<ComptimeValue>> value_refs;
                std::vector<gcref<Type>> type_refs;
                for (const auto &node: call->args) {
                    if (auto arg_attempt = cctx->try_exec(node.get(),
                                                          rctx); arg_attempt.has_value()) {
                        arguments.push_back(
                                PassedFunctionArgument{false, arg_attempt->get(), arg_attempt->get()->type});
                        value_refs.push_back(std::move(arg_attempt.value()));
                    } else {
                        // We have to do a type deduction on this one :)
                        auto arg_ty = empty_ctx->get_type(node.get());
                        arguments.push_back(PassedFunctionArgument{true, nullptr, arg_ty});
                    }
                }
                // TODO:  Use exceptions so we don't have to pass call loc
                auto function = function_set->set->get(std::move(arguments), call->location);
                return function->returnType;
            }
            NOT_IMPL_FOR(typeid(*ctime).name());
        } else {
            NOT_IMPL_FOR("non compile time deductible functions");
        }
    }

    // Might want to make this return a gcref, as it might at some point create new types, but it shouldn't
    Type *LocalContext::get_type(parser::Node *node) {
#define WHEN_NODE_IS(type, name) if (auto name = dynamic_cast<type*>(node); name)
        auto &gc = runtime->comptime->globalContext->gc;
        WHEN_NODE_IS(parser::nodes::EqualTo, pEqualTo) {
            auto pair = get_binary_type(pEqualTo->lhs.get(), pEqualTo->rhs.get());
            auto lhs_ty = pair.first;
            auto rhs_ty = pair.second;
            // Now we have to check if a type is a trivial arithmetic type
            if (trivial_arithmetic_type(lhs_ty) && trivial_arithmetic_type(rhs_ty)) {
                // Now we do the boolean type here :3
                return BooleanType::get(gc);
            } else {
                NOT_IMPL_FOR(std::string{"equal to w/ "} + lhs_ty->to_string() + " & " + rhs_ty->to_string());
            }
        }
        WHEN_NODE_IS(parser::nodes::ValueReference, pValueReference) {
            return runtime->get(pValueReference->name)->type;
        }
        WHEN_NODE_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
            if (expected_type && dynamic_cast<IntegerType *>(expected_type)) {
                return expected_type;
            } else {
                return ComptimeIntegerType::get(gc);
            }
        }
        WHEN_NODE_IS(parser::nodes::Multiplication, pMultiplication) {
            auto pair = get_binary_type(pMultiplication->lhs.get(), pMultiplication->rhs.get());
            auto lhs_ty = pair.first;
            auto rhs_ty = pair.second;
            // Now we have to check if a type is a trivial arithmetic type
            if (trivial_arithmetic_type(lhs_ty) && trivial_arithmetic_type(rhs_ty)) {
                return peer_type({lhs_ty, rhs_ty}, gc);
            } else {
                NOT_IMPL_FOR(std::string{"multiply w/ "} + lhs_ty->to_string() + " & " + rhs_ty->to_string());
            }
        }
        WHEN_NODE_IS(parser::nodes::Subtraction, pSubtraction) {
            auto pair = get_binary_type(pSubtraction->lhs.get(), pSubtraction->rhs.get());
            auto lhs_ty = pair.first;
            auto rhs_ty = pair.second;
            // Now we have to check if a type is a trivial arithmetic type
            if (trivial_arithmetic_type(lhs_ty) && trivial_arithmetic_type(rhs_ty)) {
                return peer_type({lhs_ty, rhs_ty}, gc);
            } else {
                NOT_IMPL_FOR(std::string{"subtraction w/ "} + lhs_ty->to_string() + " & " + rhs_ty->to_string());
            }
        }
        WHEN_NODE_IS(parser::nodes::TupleCall, pTupleCall) {
            // Now time to do a bunch of work to get the type of *one* function call
            return get_function_call_type(this, pTupleCall);
        }
        NOT_IMPL_FOR(typeid(*node).name());
#undef WHEN_NODE_IS
    }

    std::pair<Type *, Type *> LocalContext::get_binary_type(parser::Node *lhs, parser::Node *rhs) {
        auto &gc = runtime->comptime->globalContext->gc;
        auto lhs_ty = get_type(lhs);
        // To make this easier
        auto sub_ctx = gc.gcnew<LocalContext>(this, lhs_ty);
        auto rhs_ty = sub_ctx->get_type(rhs);
        try {
            auto peer = peer_type({lhs_ty, rhs_ty}, gc);
            auto peer_ctx = gc.gcnew<LocalContext>(this, peer);
            lhs_ty = peer_ctx->get_type(lhs);
            rhs_ty = peer_ctx->get_type(rhs);
        } catch (const NotImplementedException &ne) {
            throw;
        } catch (const std::exception &e) {

        }
        return {lhs_ty, rhs_ty};
    }
}