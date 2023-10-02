//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_VARIABLEINFO_H
#define CHEESE_VARIABLEINFO_H

#include "BacteriaType.h"

namespace cheese::bacteria {
    struct VariableInfo {
        bool constant;
        std::string name;
        TypePtr type;
    };
}
#endif //CHEESE_VARIABLEINFO_H
