//
// Created by Lexi Allen on 10/1/2023.
//
#include "bacteria/FunctionContext.h"

namespace cheese::bacteria {

    VariableInfo FunctionContext::get_mutable_variable(std::string name, TypePtr type) {
        return VariableInfo();
    }

    VariableInfo FunctionContext::get_immutable_variable(std::string name, TypePtr type) {
        return VariableInfo();
    }

    VariableInfo FunctionContext::get_temporary_variable(TypePtr type) {
        auto name = dedupe_variable_name(std::to_string(next_temporary_variable++));
    }

    std::string FunctionContext::dedupe_variable_name(std::string variableName) {
        if (all_variables.contains(variableName)) {
            auto dot = variableName.rfind('.');
            if (dot != -1) {
                
            }
        } else {
            return variableName;
        }
    }
}