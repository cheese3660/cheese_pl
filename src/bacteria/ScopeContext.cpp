//
// Created by Lexi Allen on 10/2/2023.
//

#include "bacteria/ScopeContext.h"

namespace cheese::bacteria {

    ScopeContext::ScopeContext(FunctionContext &function, llvm::BasicBlock *block, ScopeContext *parentScope)
            : function_context(function), scope_builder(llvm::IRBuilder<>(block)), current_block(block),
              parent_scope(parentScope) {
    }

    llvm::BasicBlock *ScopeContext::create_block(std::string name) {
        return llvm::BasicBlock::Create(function_context.bacteria_context->context,
                                        function_context.get_block_name(name), function_context.function);
    }

    VariableInfo *ScopeContext::get_info(std::string name) {
//        return function_context.all_variables[variable_renames[name]];
        if (variable_renames.contains(name)) {
            return function_context.all_variables[variable_renames[name]];
        } else if (parent_scope != nullptr) {
            return parent_scope->get_info(name);
        } else {
            error::raise_exiting_error("bacteria", "unknown variable " + name, {0, 0, 0},
                                       error::ErrorCode::InvalidVariableReference);
        }
    }

    VariableInfo *ScopeContext::get_mutable_variable(std::string name, TypePtr type) {
        auto result = function_context.get_mutable_variable(name, type);
        variable_renames[name] = result->name;
        return result;
    }

    VariableInfo *ScopeContext::get_immutable_variable(std::string name, TypePtr type) {
        auto result = function_context.get_immutable_variable(name, type);
        variable_renames[name] = result->name;
        return result;
    }

    VariableInfo *ScopeContext::get_temporary_variable(TypePtr type) {
        auto result = function_context.get_temporary_variable(type);
        return result;
    }

    VariableInfo *ScopeContext::get_temporary_in_memory_variable(TypePtr type) {
        auto result = function_context.get_temporary_in_memory_variable(type);
        return result;
    }

    void ScopeContext::set_current_block(llvm::BasicBlock *block) {
        current_block = block;
        scope_builder.SetInsertPoint(block);
    }
}