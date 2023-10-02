//
// Created by Lexi Allen on 10/2/2023.
//

#include "bacteria/ScopeContext.h"

namespace cheese::bacteria {

    ScopeContext::ScopeContext(FunctionContext &function, llvm::BasicBlock *block, ScopeContext *parentScope)
            : function_context(function), scope_builder(llvm::IRBuilder<>(block)), current_block(block),
              parent_scope(parentScope) {
    }
}