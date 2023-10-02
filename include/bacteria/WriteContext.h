//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_WRITECONTEXT_H
#define CHEESE_WRITECONTEXT_H

#include "BacteriaType.h"
#include <string>

namespace cheese::bacteria {
    struct WriteContext {
        std::string result_variable;
        TypePtr result_type;
    };
}
#endif //CHEESE_WRITECONTEXT_H
