//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_VARIABLEINFO_H
#define CHEESE_VARIABLEINFO_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include "BacteriaType.h"

namespace cheese::bacteria {
    struct FunctionContext;

    struct VariableInfo {
        bool constant;
        std::string name;
        TypePtr type;
        // Used for generating address_of expressions for constant variables,
        // since we can't know if these addresses are going to be used safely,
        // we have to read from the address once this is set on a constant,
        // because the value might still change
        llvm::Value *value = nullptr; // On an array this becomes the array decay value
        llvm::Value *ptr = nullptr;

        llvm::Value *load_value(llvm::IRBuilder<> &builder, project::GlobalContext *ctx);

        llvm::Value *get_addr(FunctionContext &context);
    };
}
#endif //CHEESE_VARIABLEINFO_H
