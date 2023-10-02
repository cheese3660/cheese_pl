//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_FUNCTIONINFO_H
#define CHEESE_FUNCTIONINFO_H

#include "BacteriaType.h"

namespace cheese::bacteria {
    struct FunctionInfo {
        TypeList argumentTypes;
        TypePtr returnType;
        std::string name;
    };
}
#endif //CHEESE_FUNCTIONINFO_H
