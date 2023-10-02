//
// Created by Lexi Allen on 10/1/2023.
//

#include "bacteria/nodes/expression_nodes.h"
#include "bacteria/BacteriaContext.h"
#include "bacteria/ScopeContext.h"
#include "bacteria/ExpressionContext.h"
#include "curdle/curdle.h"
#include "bacteria/FunctionContext.h"

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

    llvm::Value *CastNode::lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) {
//        return BacteriaNode::lower_expression_level(ctx, expr);
        auto lhs_ty = lhs->get_expr_type();
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

    TypePtr StringLiteral::get_expr_type() {
        return type;
    }
}