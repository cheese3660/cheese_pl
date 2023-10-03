//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_SCOPECONTEXT_H
#define CHEESE_SCOPECONTEXT_H

#include "FunctionContext.h"

namespace cheese::bacteria {
    struct ScopeContext {
        std::unordered_map<std::string, std::string> variable_renames; // This is used for getting a variable from the scope
        llvm::BasicBlock *current_block;
        llvm::IRBuilder<> scope_builder;
        ScopeContext *parent_scope;
        FunctionContext &function_context;

        VariableInfo get_info(std::string name);

        VariableInfo get_mutable_variable(std::string name, TypePtr type);

        VariableInfo get_immutable_variable(std::string name, TypePtr type);

        VariableInfo get_temporary_variable(TypePtr type);

        VariableInfo get_temporary_in_memory_variable(TypePtr type);

        void
        set_current_block(llvm::BasicBlock *block); // This will also set the IRBuilder for the scope to use this block

        ScopeContext(FunctionContext &function, llvm::BasicBlock *block, ScopeContext *parentScope = nullptr);

        llvm::BasicBlock *create_block(std::string name);
    };
}

#endif //CHEESE_SCOPECONTEXT_H
