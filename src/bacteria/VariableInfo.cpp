//
// Created by Lexi Allen on 10/2/2023.
//
#include "bacteria/VariableInfo.h"
#include "bacteria/FunctionContext.h"

namespace cheese::bacteria {

    llvm::Value *VariableInfo::load_value(llvm::IRBuilder<> &builder, project::GlobalContext *ctx) {
        if (type->type == BacteriaType::Type::Array) {
            return value; // This is just the array decay
        } else {
            if (constant && !ptr) {
                return value;
            } else if (constant) {
                return builder.CreateLoad(type->get_llvm_type(ctx), ptr);
            } else {
                return builder.CreateLoad(type->get_llvm_type(ctx), value);
            }
        }
    }

    llvm::Value *VariableInfo::get_addr(FunctionContext &context) {
        return context.get_variable_address(*this);
    }
}