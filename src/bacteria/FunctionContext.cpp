//
// Created by Lexi Allen on 10/1/2023.
//
#include <utility>

#include "bacteria/FunctionContext.h"

namespace cheese::bacteria {

    VariableInfo *FunctionContext::get_mutable_variable(std::string varName, TypePtr type) {
        auto name = dedupe_variable_name(std::move(varName));
        auto info = new VariableInfo{
                false,
                name,
                type
        };
        all_variables[name] = info;
        info->value = allocate(name, type);
        return info;
    }

    VariableInfo *FunctionContext::get_immutable_variable(std::string varName, TypePtr type) {
        auto name = dedupe_variable_name(std::move(varName));
        auto info = new VariableInfo{
                true,
                name,
                type
        };
        all_variables[name] = info;
        return info;
    }

    VariableInfo *FunctionContext::get_temporary_variable(TypePtr type) {
        auto name = dedupe_variable_name(std::to_string(next_temporary_variable++));
        auto info = new VariableInfo{
                true,
                name,
                type
        };
        all_variables[name] = info;
        return info;
    }

    VariableInfo *FunctionContext::get_temporary_in_memory_variable(TypePtr type) {
        auto name = dedupe_variable_name(std::to_string(next_temporary_variable++));
        auto info = new VariableInfo{
                false,
                name,
                type
        };
        all_variables[name] = info;
        info->value = allocate(name, type);
        return info;
    }

    static bool is_number(const std::string &s) {
        return !s.empty() && std::find_if(s.begin(),
                                          s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }

    std::string FunctionContext::dedupe_variable_name(std::string variableName) {
        if (all_variables.contains(variableName)) {
            auto dot = variableName.rfind('.');
            if (dot != -1) {
                auto last = variableName.substr(dot + 1);
                if (is_number(last)) {
                    auto num = std::stoi(last);
                    return dedupe_variable_name(variableName.substr(0, dot) + "." + std::to_string(num + 1));
                } else {
                    return dedupe_variable_name(variableName + ".0");
                }
            } else {
                return dedupe_variable_name(variableName + ".0");
            }
        } else {
            return variableName;
        }
    }

    FunctionContext::FunctionContext(BacteriaContext *bacteriaContext, llvm::Function *function) : function(function),
                                                                                                   bacteria_context(
                                                                                                           bacteriaContext) {
        entry_block = llvm::BasicBlock::Create(bacteriaContext->context, get_block_name(".entry"), function);
        allocation_block = llvm::BasicBlock::Create(bacteriaContext->context, get_block_name(".var_alloc"), function,
                                                    entry_block);

        auto irBuilder = llvm::IRBuilder<>(bacteria_context->context);
        irBuilder.SetInsertPoint(allocation_block);
        goto_entry_instruction = irBuilder.CreateBr(entry_block);
    }

    llvm::Value *FunctionContext::allocate(const std::string &name, TypePtr type) {
        auto irBuilder = llvm::IRBuilder<>(bacteria_context->context);
        irBuilder.SetInsertPoint(goto_entry_instruction);
        auto alloc = irBuilder.CreateAlloca(type->get_llvm_type(bacteria_context->global_context), nullptr, name);
        return alloc;
    }

    std::string FunctionContext::get_block_name(std::string wantedName) {
        if (all_block_names.contains(wantedName)) {
            auto dot = wantedName.rfind('.');
            if (dot != -1) {
                auto last = wantedName.substr(dot + 1);
                if (is_number(last)) {
                    auto num = std::stoi(last);
                    return get_block_name(wantedName.substr(0, dot) + "." + std::to_string(num + 1));
                } else {
                    return get_block_name(wantedName + ".0");
                }
            } else {
                return get_block_name(wantedName + ".0");
            }
        } else {
            all_block_names.insert(wantedName);
            return wantedName;
        }
    }

    llvm::Value *FunctionContext::get_variable_address(VariableInfo &info) {
        if (!info.constant || info.type->type == BacteriaType::Type::Array) return info.value;
        if (info.ptr) return info.ptr;
        auto full_name = info.name + ".addr";
        auto irBuilder = llvm::IRBuilder<>(bacteria_context->context);
        irBuilder.SetInsertPoint(goto_entry_instruction);
        auto inst = irBuilder.CreateAlloca(info.type->get_llvm_type(bacteria_context->global_context), nullptr,
                                           full_name);
        irBuilder.CreateStore(info.value, inst, false);
        info.ptr = inst;
        return info.ptr;
    }

    FunctionContext::~FunctionContext() {
        for (const auto &var: all_variables) {
            delete var.second;
        }
    }

}