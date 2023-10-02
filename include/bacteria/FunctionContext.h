//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_FUNCTIONCONTEXT_H
#define CHEESE_FUNCTIONCONTEXT_H

#include <unordered_map>
#include <string>
#include <llvm/IR/IRBuilder.h>
#include "VariableInfo.h"

namespace cheese::bacteria {
    struct FunctionContext {
        std::unordered_map<std::string, VariableInfo> all_variables;
        std::size_t next_temporary_variable{0};
        llvm::Function *function;


        VariableInfo get_mutable_variable(std::string name, TypePtr type);

        VariableInfo get_immutable_variable(std::string name, TypePtr type);

        VariableInfo get_temporary_variable(TypePtr type);

        std::string dedupe_variable_name(std::string variableName);
    };
}
#endif //CHEESE_FUNCTIONCONTEXT_H
