//
// Created by Lexi Allen on 10/1/2023.
//

#include "bacteria/nodes/expression_nodes.h"
#include "bacteria/BacteriaContext.h"
#include "bacteria/ScopeContext.h"
#include "bacteria/ExpressionContext.h"
#include "curdle/curdle.h"
#include "bacteria/FunctionContext.h"
#include "bacteria/nodes/receiver_nodes.h"

namespace cheese::bacteria::nodes {

    void FunctionImport::gen_protos(cheese::bacteria::BacteriaContext *ctx) {
        auto returnType = return_type->get_llvm_type(ctx->global_context);
        std::vector<llvm::Type *> argTypes;
        for (auto &arg: arguments) {
            argTypes.push_back(arg->get_llvm_type(ctx->global_context));
        }
        auto functionType = llvm::FunctionType::get(returnType, argTypes, false);
        auto prototype = llvm::Function::Create(functionType, llvm::Function::AvailableExternallyLinkage, name,
                                                ctx->program_module);
        ctx->functions[name] = FunctionInfo{
                arguments,
                return_type,
                name,
                prototype
        };
    }

    void FunctionImport::lower_top_level(cheese::bacteria::BacteriaContext *ctx) {

    }

    void Return::lower_scope_level(ScopeContext &ctx) {
        auto expected_type = ctx.function_context.return_type;
        if (retVal.has_value()) {
            ExpressionContext expressionContext{
                    expected_type
            };
            auto value = retVal.value()->lower_expression_level(ctx, expressionContext);
            ctx.scope_builder.CreateRet(value);
        } else {
            if (expected_type->type != BacteriaType::Type::Void) {
                throw curdle::LocalizedCurdleError{
                        "Invalid return: returning void from a non void function",
                        location,
                        error::ErrorCode::InvalidReturn
                };
            }
            ctx.scope_builder.CreateRetVoid();
        }
    }

    llvm::Value *NormalCallNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto fn = ctx.function_context.bacteria_context->functions[function];
        std::vector<llvm::Value *> args;
        size_t idx = 0;
        for (auto &arg: arguments) {
            ExpressionContext expressionContext{
                    fn.argumentTypes[idx++]
            };
            args.push_back(arg->lower_expression_level(ctx, expressionContext));
        }
        return ctx.scope_builder.CreateCall(fn.prototype->getFunctionType(), fn.prototype, args);
    }

    void NormalCallNode::lower_scope_level(ScopeContext &ctx) {
        ExpressionContext voidContext{
                ctx.function_context.bacteria_context->program->get_type(bacteria::BacteriaType::Type::Void)
        };
        lower_expression_level(ctx, voidContext);
    }

    TypePtr NormalCallNode::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        auto fn = ctx.function_context.bacteria_context->functions[function];
        return fn.returnType;
    }

    llvm::Value *CastNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
//        return BacteriaNode::lower_expression_level(ctx, expr);
        auto lhs_ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        if (lhs_ty->is_same_as(rhs)) {
            return lhs->lower_expression_level(ctx, expr);
        } else {
            auto lhsValue = lhs->lower_expression_level(ctx, expr);
            auto rhsTy = rhs->get_llvm_type(
                    ctx.function_context.bacteria_context->global_context);
            switch (lhs_ty->type) {
                case BacteriaType::Type::UnsignedInteger:
                    switch (rhs->type) {
                        case BacteriaType::Type::UnsignedInteger:
                            if (lhs_ty->integer_size < rhs->integer_size) {
                                return ctx.scope_builder.CreateZExt(lhsValue, rhsTy);
                            } else {
                                return ctx.scope_builder.CreateTrunc(lhsValue, rhsTy);
                            }
                            break;
                        case BacteriaType::Type::SignedInteger:
                            if (lhs_ty->integer_size < rhs->integer_size) {
                                return ctx.scope_builder.CreateSExt(lhsValue, rhsTy);
                            } else {
                                return ctx.scope_builder.CreateTrunc(lhsValue, rhsTy);
                            }
                            break;
                    }
                    break;
                case BacteriaType::Type::SignedInteger:
                    switch (rhs->type) {
                        case BacteriaType::Type::UnsignedInteger:
                        case BacteriaType::Type::SignedInteger:
                            if (lhs_ty->integer_size < rhs->integer_size) {
                                return ctx.scope_builder.CreateSExt(lhsValue, rhsTy);
                            } else {
                                return ctx.scope_builder.CreateTrunc(lhsValue, rhsTy);
                            }
                            break;
                    }
                    break;
                case BacteriaType::Type::Array:
                    switch (rhs->type) {
                        case BacteriaType::Type::Pointer:
                            return lhsValue;
                    }
            }
            NOT_IMPL_FOR(lhs_ty->to_string() + " & " + rhs->to_string());
        }
    }

    llvm::Value *StringLiteral::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        if (type->type == BacteriaType::Type::Pointer) {
            return ctx.function_context.bacteria_context->get_string_constant(value);
        } else {
            NOT_IMPL;
        }
    }

    TypePtr StringLiteral::get_expr_type(ScopeContext &ctx, BacteriaProgram *program) {
        return type;
    }

    void If::lower_scope_level(ScopeContext &ctx) {
        // Here we don't care about values
        ExpressionContext expressionContext{
                condition->get_expr_type(ctx, ctx.function_context.bacteria_context->program)
        };
        auto comparison = condition->lower_expression_level(ctx, expressionContext);
        auto if_block = ctx.create_block(".if-true");
        auto if_scope = ScopeContext{
                ctx.function_context,
                if_block,
                &ctx
        };
        ExpressionContext voidContext{
                ctx.function_context.bacteria_context->program->get_type(bacteria::BacteriaType::Type::Void)
        };
        body->lower_expression_level(if_scope, voidContext);
        if (els.has_value()) {
            auto else_block = ctx.create_block(".if-false");
            auto else_scope = ScopeContext{
                    ctx.function_context,
                    else_block,
                    &ctx
            };
            els.value()->lower_expression_level(else_scope, voidContext);
            if (!util::llvm::has_terminator(ctx.current_block)) {
                ctx.scope_builder.CreateCondBr(comparison, if_block, else_block);
            }
            auto cont_block = ctx.create_block(".cont");
            if (!util::llvm::has_terminator(if_scope.current_block)) {
                if_scope.scope_builder.CreateBr(cont_block);
            }
            if (!util::llvm::has_terminator(else_scope.current_block)) {
                else_scope.scope_builder.CreateBr(cont_block);
            }
            ctx.set_current_block(cont_block);
        } else {
            auto cont_block = ctx.create_block(".cont");
            if (!util::llvm::has_terminator(ctx.current_block)) {
                ctx.scope_builder.CreateCondBr(comparison, if_block, cont_block);
            }
            if (!util::llvm::has_terminator(if_scope.current_block)) {
                if_scope.scope_builder.CreateBr(cont_block);
            }
            ctx.set_current_block(cont_block);
        }
    }

    void While::lower_scope_level(ScopeContext &ctx) {
        ExpressionContext expressionContext{
                condition->get_expr_type(ctx, ctx.function_context.bacteria_context->program)
        };
        auto compare_block = ctx.create_block(".while-compare");
        auto compare_scope = ScopeContext{
                ctx.function_context,
                compare_block,
                &ctx
        };
        auto comparison = condition->lower_expression_level(compare_scope, expressionContext);
        auto while_block = ctx.create_block(".while-loop");
        auto while_scope = ScopeContext{
                ctx.function_context,
                while_block,
                &ctx
        };
        ExpressionContext voidContext{
                ctx.function_context.bacteria_context->program->get_type(bacteria::BacteriaType::Type::Void)
        };
        body->lower_expression_level(while_scope, voidContext);
        if (els.has_value()) {
            auto else_block = ctx.create_block(".while-else");
            auto else_scope = ScopeContext{
                    ctx.function_context,
                    else_block,
                    &ctx
            };
            els.value()->lower_expression_level(else_scope, voidContext);
            if (!util::llvm::has_terminator(ctx.current_block)) {
                ctx.scope_builder.CreateBr(compare_block);
            }
            if (!util::llvm::has_terminator(while_scope.current_block)) {
                while_scope.scope_builder.CreateBr(compare_block);
            }
            if (!util::llvm::has_terminator(compare_scope.current_block)) {
                // We are so going to have to create a `break block` at some point
                compare_scope.scope_builder.CreateCondBr(comparison, while_block, else_block);
            }
            auto cont_block = ctx.create_block(".cont");
            if (!util::llvm::has_terminator(else_scope.current_block)) {
                else_scope.scope_builder.CreateBr(cont_block);
            }
            ctx.set_current_block(cont_block);
        } else {
            auto cont_block = ctx.create_block(".cont");
            if (!util::llvm::has_terminator(ctx.current_block)) {
                ctx.scope_builder.CreateBr(compare_block);
            }
            if (!util::llvm::has_terminator(while_scope.current_block)) {
                while_scope.scope_builder.CreateBr(compare_block);
            }
            if (!util::llvm::has_terminator(compare_scope.current_block)) {
                // We are so going to have to create a `break block` at some point
                compare_scope.scope_builder.CreateCondBr(comparison, while_block, cont_block);
            }
            ctx.set_current_block(cont_block);
        }
    }

    llvm::Value *LesserThanNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
                return ctx.scope_builder.CreateICmpULT(lhsValue, rhsValue);
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateICmpSLT(lhsValue, rhsValue);
            case BacteriaType::Type::Pointer:
            case BacteriaType::Type::FunctionPointer: {
                auto zero = ctx.scope_builder.getIntN(
                        ctx.function_context.bacteria_context->global_context->machine.data_pointer_size * 8, 0);
                auto ptrDiff = ctx.scope_builder.CreatePtrDiff(
                        llvm::Type::getInt8Ty(ctx.function_context.bacteria_context->context), lhsValue, rhsValue);
                return ctx.scope_builder.CreateICmpSLT(ptrDiff, zero);
            }
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do < values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    TypePtr LesserThanNode::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return program->get_type(BacteriaType::Type::UnsignedInteger, 1);
    }

    llvm::Value *EqualToNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateICmpEQ(lhsValue, rhsValue);
            case BacteriaType::Type::Pointer:
            case BacteriaType::Type::FunctionPointer: {
                auto zero = ctx.scope_builder.getIntN(
                        ctx.function_context.bacteria_context->global_context->machine.data_pointer_size * 8, 0);
                auto ptrDiff = ctx.scope_builder.CreatePtrDiff(
                        llvm::Type::getInt8Ty(ctx.function_context.bacteria_context->context), lhsValue, rhsValue);
                return ctx.scope_builder.CreateICmpEQ(ptrDiff, zero);
            }
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do == values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    TypePtr EqualToNode::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return program->get_type(BacteriaType::Type::UnsignedInteger, 1);
    }

    llvm::Value *NotEqualNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateICmpNE(lhsValue, rhsValue);
            case BacteriaType::Type::Pointer:
            case BacteriaType::Type::FunctionPointer: {
                auto zero = ctx.scope_builder.getIntN(
                        ctx.function_context.bacteria_context->global_context->machine.data_pointer_size * 8, 0);
                auto ptrDiff = ctx.scope_builder.CreatePtrDiff(
                        llvm::Type::getInt8Ty(ctx.function_context.bacteria_context->context), lhsValue, rhsValue);
                return ctx.scope_builder.CreateICmpNE(ptrDiff, zero);
            }
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do != values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    TypePtr NotEqualNode::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return program->get_type(BacteriaType::Type::UnsignedInteger, 1);
    }

    llvm::Value *GreaterEqualNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
                return ctx.scope_builder.CreateICmpUGE(lhsValue, rhsValue);
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateICmpSGE(lhsValue, rhsValue);
            case BacteriaType::Type::Pointer:
            case BacteriaType::Type::FunctionPointer: {
                auto zero = ctx.scope_builder.getIntN(
                        ctx.function_context.bacteria_context->global_context->machine.data_pointer_size * 8, 0);
                auto ptrDiff = ctx.scope_builder.CreatePtrDiff(
                        llvm::Type::getInt8Ty(ctx.function_context.bacteria_context->context), lhsValue, rhsValue);
                return ctx.scope_builder.CreateICmpSGE(ptrDiff, zero);
            }
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do >= values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    TypePtr GreaterEqualNode::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return program->get_type(BacteriaType::Type::UnsignedInteger, 1);
    }

    llvm::Value *AdditionNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateAdd(lhsValue, rhsValue);
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do + values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    llvm::Value *SubtractNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateSub(lhsValue, rhsValue);
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do - values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    llvm::Value *ModulusNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
                return ctx.scope_builder.CreateURem(lhsValue, rhsValue);
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateSRem(lhsValue, rhsValue);
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do % values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    llvm::Value *DivisionNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        auto subContext = ExpressionContext{
                ty
        };
        auto lhsValue = lhs->lower_expression_level(ctx, subContext);
        auto rhsValue = rhs->lower_expression_level(ctx, subContext);
        switch (ty->type) {
            case BacteriaType::Type::UnsignedInteger:
                return ctx.scope_builder.CreateUDiv(lhsValue, rhsValue);
            case BacteriaType::Type::SignedInteger:
                return ctx.scope_builder.CreateSDiv(lhsValue, rhsValue);
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do / values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    llvm::Value *ValueReference::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        return ctx.get_info(name)->load_value(ctx.scope_builder, ctx.function_context.bacteria_context->global_context);
    }

    TypePtr ValueReference::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return ctx.get_info(name)->type;
    }

    llvm::Value *ValueReference::lower_address(ScopeContext &ctx) {
        return ctx.get_info(name)->get_addr(ctx.function_context);
    }

    llvm::Value *IntegerLiteral::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto apInt = llvm::APInt(type->integer_size, static_cast<std::string>(value), 10);
//        return llvm::ConstantInt::get(type->get_llvm_type(ctx.function_context.bacteria_context->global_context),
//                                      apInt);
        return ctx.scope_builder.getInt(apInt);
    }

    TypePtr IntegerLiteral::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return BacteriaNode::get_expr_type(ctx, program);
    }

    TypePtr ArrayIndexNode::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return array->get_expr_type(ctx, program)->index_type(program, arguments.size());
    }

    llvm::Value *ArrayIndexNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        auto arr_type = array->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        switch (arr_type->type) {
            case BacteriaType::Type::Pointer: {
                auto subtype = arr_type->subtype;
                auto subtypeLLVM = subtype->get_llvm_type(ctx.function_context.bacteria_context->global_context);
                ExpressionContext usizeContext{
                        ctx.function_context.bacteria_context->program->get_type(BacteriaType::Type::UnsignedInteger,
                                                                                 ctx.function_context.bacteria_context->global_context->machine.data_pointer_size)
                };
                ExpressionContext arrayContext{
                        arr_type
                };
                auto ptr = array->lower_expression_level(ctx, arrayContext);
                auto index = arguments[0]->lower_expression_level(ctx, usizeContext);

                auto ep = ctx.scope_builder.CreateGEP(subtypeLLVM, ptr, {index});
                if (subtype->type != BacteriaType::Type::Array) { // We don't want to load arrays ...
                    return ctx.scope_builder.CreateLoad(subtypeLLVM, ep);
                } else {
                    return ep;
                }
            }
            case BacteriaType::Type::Array: {
                auto subtype = arr_type->index_type(ctx.function_context.bacteria_context->program, arguments.size());
                auto subtypeLLVM = subtype->get_llvm_type(ctx.function_context.bacteria_context->global_context);
                ExpressionContext usizeContext{
                        ctx.function_context.bacteria_context->program->get_type(BacteriaType::Type::UnsignedInteger,
                                                                                 ctx.function_context.bacteria_context->global_context->machine.data_pointer_size)
                };
                ExpressionContext arrayContext{
                        arr_type
                };
                auto ptr = array->lower_expression_level(ctx, arrayContext);
                std::vector<llvm::Value *> indices;
                for (const auto &child: arguments) {
                    indices.push_back(child->lower_expression_level(ctx, usizeContext));
                }
                auto ep = ctx.scope_builder.CreateGEP(
                        arr_type->get_llvm_type(ctx.function_context.bacteria_context->global_context), ptr, indices);
                if (subtype->type != BacteriaType::Type::Array &&
                    indices.size() >= arr_type->array_dimensions.size()) { // We don't want to load arrays ...
                    return ctx.scope_builder.CreateLoad(subtypeLLVM, ep);
                } else {
                    return ep;
                }
            }
            default:
                throw LocalizedCurdleError{
                        "Invalid Index: Cannot index values of type " + arr_type->to_string(),
                        location,
                        error::ErrorCode::InvalidIndex
                };
        }

    }

    llvm::Value *ArrayIndexNode::lower_address(ScopeContext &ctx) {
        auto arr_type = array->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        switch (arr_type->type) {
            case BacteriaType::Type::Pointer: {
                auto subtype = arr_type->subtype;
                auto subtypeLLVM = subtype->get_llvm_type(ctx.function_context.bacteria_context->global_context);
                ExpressionContext usizeContext{
                        ctx.function_context.bacteria_context->program->get_type(BacteriaType::Type::UnsignedInteger,
                                                                                 ctx.function_context.bacteria_context->global_context->machine.data_pointer_size)
                };
                ExpressionContext arrayContext{
                        arr_type
                };
                auto ptr = array->lower_expression_level(ctx, arrayContext);
                auto index = arguments[0]->lower_expression_level(ctx, usizeContext);

                auto ep = ctx.scope_builder.CreateGEP(subtypeLLVM, ptr, {index});
                return ep;
            }
            case BacteriaType::Type::Array: {
                auto subtype = arr_type->index_type(ctx.function_context.bacteria_context->program, arguments.size());
                auto subtypeLLVM = subtype->get_llvm_type(ctx.function_context.bacteria_context->global_context);
                ExpressionContext usizeContext{
                        ctx.function_context.bacteria_context->program->get_type(BacteriaType::Type::UnsignedInteger,
                                                                                 ctx.function_context.bacteria_context->global_context->machine.data_pointer_size)
                };
                ExpressionContext arrayContext{
                        arr_type
                };
                auto ptr = array->lower_expression_level(ctx, arrayContext);
                std::vector<llvm::Value *> indices;
                for (const auto &child: arguments) {
                    indices.push_back(child->lower_expression_level(ctx, usizeContext));
                }
                auto ep = ctx.scope_builder.CreateGEP(
                        arr_type->get_llvm_type(ctx.function_context.bacteria_context->global_context), ptr, indices);
                return ep;
            }
            default:
                throw LocalizedCurdleError{
                        "Invalid Index: Cannot index values of type " + arr_type->to_string(),
                        location,
                        error::ErrorCode::InvalidIndex
                };
        }
    }

    void VariableInitializationNode::lower_scope_level(ScopeContext &ctx) {
        ExpressionContext valueContext{
                type
        };
        auto val = value->lower_expression_level(ctx, valueContext);
        if (constant) {
            auto info = ctx.get_immutable_variable(name, type);
            val->setName(info->name);
            info->value = val;
        } else {
            auto info = ctx.get_mutable_variable(name, type);
            ctx.scope_builder.CreateStore(val, info->value);
        }
    }

    void VariableDefinitionNode::lower_scope_level(ScopeContext &ctx) {
//        BacteriaNode::lower_scope_level(ctx);
        ctx.get_mutable_variable(name, type);
    }


    void MutationNode::lower_scope_level(ScopeContext &ctx) {
        auto lhs_addr = lhs->lower_address(ctx);
        auto lhs_ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        ExpressionContext expressionContext{
                lhs_ty
        };
        ctx.scope_builder.CreateStore(rhs->lower_expression_level(ctx, expressionContext), lhs_addr);
    }


}