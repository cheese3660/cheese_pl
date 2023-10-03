//
// Created by Lexi Allen on 9/26/2023.
//

#ifndef CHEESE_LLVM_UTILS_H
#define CHEESE_LLVM_UTILS_H

#include <llvm/IR/BasicBlock.h>

namespace cheese::util::llvm {
    void initialize_llvm();

    bool has_terminator(::llvm::BasicBlock *basicBlock);
}
#endif //CHEESE_LLVM_UTILS_H
