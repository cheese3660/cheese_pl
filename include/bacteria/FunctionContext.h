//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_FUNCTIONCONTEXT_H
#define CHEESE_FUNCTIONCONTEXT_H

#include <unordered_map>
#include <set>
#include <string>
#include <llvm/IR/IRBuilder.h>
#include "VariableInfo.h"
#include "BacteriaContext.h"

namespace cheese::bacteria {
    struct FunctionContext {
        TypePtr return_type;
        std::unordered_map<std::string, VariableInfo> all_variables;
        std::set<std::string> all_block_names;
        std::size_t next_temporary_variable{0};
        llvm::Function *function;
        llvm::BasicBlock *allocation_block;
        llvm::BasicBlock *entry_block;
        llvm::Instruction *goto_entry_instruction;
        BacteriaContext *bacteria_context;

        VariableInfo get_mutable_variable(std::string name, TypePtr type);

        VariableInfo get_immutable_variable(std::string name, TypePtr type);

        VariableInfo get_temporary_variable(TypePtr type);

        VariableInfo get_temporary_in_memory_variable(TypePtr type);

        std::string dedupe_variable_name(std::string variableName);

        llvm::Value *allocate(const std::string &name, TypePtr type);

        FunctionContext(BacteriaContext *bacteriaContext, llvm::Function *function);

        std::string get_block_name(std::string wantedName);

        llvm::Value *get_variable_address(VariableInfo &info);
    };
}
#endif //CHEESE_FUNCTIONCONTEXT_H
