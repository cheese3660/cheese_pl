//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_BACTERIACONTEXT_H
#define CHEESE_BACTERIACONTEXT_H

#include "project/GlobalContext.h"
#include <llvm/IR/IRBuilder.h>
#include "FunctionInfo.h"
#include "VariableInfo.h"

namespace cheese::bacteria {
    using namespace memory::garbage_collection;

    struct BacteriaContext : managed_object {
        project::GlobalContext *global_context;
        llvm::Module *program_module;
        llvm::IRBuilder<> *ir_builder;
        llvm::LLVMContext &context;
        std::unordered_map<std::string, std::string> string_constants;
        std::unordered_map<std::string, FunctionInfo> functions;
        std::unordered_map<std::string, VariableInfo> global_variables;
        std::size_t next_string_constant_name{0};

        void mark_references() override;

        ~BacteriaContext() override;

        explicit BacteriaContext(project::GlobalContext *globalContext);

        std::string get_string_constant(std::string constant); // This returns a ptr u8

    };
}
#endif //CHEESE_BACTERIACONTEXT_H
