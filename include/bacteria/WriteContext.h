//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_WRITECONTEXT_H
#define CHEESE_WRITECONTEXT_H

#include "BacteriaType.h"
#include <string>
#include <llvm/IR/Value.h>

namespace cheese::bacteria {
    struct WriteContext {
        llvm::Value *result_valuee;
        TypePtr result_type;
    };
}
#endif //CHEESE_WRITECONTEXT_H
