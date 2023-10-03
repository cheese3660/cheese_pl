//
// Created by Lexi Allen on 10/1/2023.
//
#include "bacteria/BacteriaContext.h"
#include "bacteria/nodes/receiver_nodes.h"

void cheese::bacteria::BacteriaContext::mark_references() {
    global_context->mark();
}

cheese::bacteria::BacteriaContext::BacteriaContext(cheese::project::GlobalContext *globalContext,
                                                   nodes::BacteriaProgram *program) : global_context(
        globalContext), context(globalContext->llvm_context), program(program) {
    program_module = new llvm::Module("main", globalContext->llvm_context);
    program_module->setDataLayout(globalContext->machine.layout);
    program_module->setSourceFileName(globalContext->project.root_path.filename().string());
}

llvm::Value *cheese::bacteria::BacteriaContext::get_string_constant(std::string constant) {
    if (string_constants.contains(constant)) return string_constants[constant];
    auto next_name = ".str__" + std::to_string(next_string_constant_name++);
    auto llvmConstant = program_module->getOrInsertGlobal(next_name,
                                                          llvm::ArrayType::get(llvm::Type::getInt8Ty(context),
                                                                               constant.size() + 1));
    auto globalVariable = program_module->getNamedGlobal(next_name);
    globalVariable->setConstant(true);
    globalVariable->setLinkage(llvm::GlobalValue::PrivateLinkage);
    globalVariable->setInitializer(llvm::ConstantDataArray::getString(context, constant, true));
    string_constants[constant] = globalVariable;
    return globalVariable;
}

cheese::bacteria::BacteriaContext::~BacteriaContext() {
}
