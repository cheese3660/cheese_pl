//
// Created by Lexi Allen on 10/2/2023.
//
#include "bacteria/VariableInfo.h"

namespace cheese::bacteria {

    llvm::Value *VariableInfo::load_value(llvm::IRBuilder<> &builder, project::GlobalContext *ctx) {
        if (constant && !ptr) {
            return value;
        } else if (constant) {
            return builder.CreateLoad(type->get_llvm_type(ctx), ptr);
        } else {
            return builder.CreateLoad(type->get_llvm_type(ctx), value);
        }
    }
}