//
// Created by Lexi Allen on 9/26/2023.
//
#include "util/llvm_utils.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

void cheese::util::llvm::initialize_llvm() {
    static bool llvm_initialized = false;
    if (!llvm_initialized) {
        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();
        llvm_initialized = true;
    }
}

