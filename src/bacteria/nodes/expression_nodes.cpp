//
// Created by Lexi Allen on 10/1/2023.
//

#include "bacteria/nodes/expression_nodes.h"
#include "bacteria/BacteriaContext.h"


namespace cheese::bacteria::nodes {

    void FunctionImport::lower_top_level(BacteriaContext *ctx) {
        auto returnType = return_type->get_llvm_type(ctx->global_context);
        std::vector<llvm::Type *> argTypes;
        for (auto &arg: arguments) {
            argTypes.push_back(arg->get_llvm_type(ctx->global_context));
        }
        auto functionType = llvm::FunctionType::get(returnType, argTypes, false);
        auto prototype = llvm::Function::Create(functionType, llvm::Function::AvailableExternallyLinkage, name,
                                                ctx->program_module);
    }
}