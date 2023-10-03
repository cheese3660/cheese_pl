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

    llvm::Value *CastNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
//        return BacteriaNode::lower_expression_level(ctx, expr);
        auto lhs_ty = lhs->get_expr_type(ctx, ctx.function_context.bacteria_context->program);
        if (lhs_ty->is_same_as(rhs)) {
            return lhs->lower_expression_level(ctx, expr);
        } else {
            NOT_IMPL;
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
            ctx.scope_builder.CreateCondBr(comparison, if_block, else_block);
            auto cont_block = ctx.create_block(".if-cont");
            if_scope.scope_builder.CreateBr(cont_block);
            else_scope.scope_builder.CreateBr(cont_block);
            ctx.set_current_block(cont_block);
        } else {
            auto cont_block = ctx.create_block(".if-cont");
            ctx.scope_builder.CreateCondBr(comparison, if_block, cont_block);
            if_scope.scope_builder.CreateBr(cont_block);
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
//                auto zero = llvm::ConstantInt::get(llvm::Type::getIntNTy(ctx.function_context.bacteria_context->context,
//                                                                         ctx.function_context.bacteria_context->global_context->machine.data_pointer_size *
//                                                                         8), 0);
                auto zero = ctx.scope_builder.getIntN(
                        ctx.function_context.bacteria_context->global_context->machine.data_pointer_size * 8, 0);
                auto ptrDiff = ctx.scope_builder.CreatePtrDiff(
                        llvm::Type::getInt8Ty(ctx.function_context.bacteria_context->context), lhsValue, rhsValue);
                return ctx.scope_builder.CreateICmpSLT(ptrDiff, zero);
            }
            default:
                throw LocalizedCurdleError{
                        "Invalid Comparison: Cannot do <= values of type " + ty->to_string(),
                        location,
                        error::ErrorCode::InvalidComparison
                };
        }
    }

    TypePtr LesserThanNode::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return program->get_type(BacteriaType::Type::UnsignedInteger, 1);
    }

    llvm::Value *ValueReference::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        return ctx.get_info(name).load_value(ctx.scope_builder, ctx.function_context.bacteria_context->global_context);
    }

    TypePtr ValueReference::get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) {
        return ctx.get_info(name).type;
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
        return BacteriaNode::get_expr_type(ctx, program);
    }

    llvm::Value *ArrayIndexNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
        return BacteriaNode::lower_expression_level(ctx, expr);
    }
}