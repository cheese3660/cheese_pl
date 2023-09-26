//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_VARIABLES_H
#define CHEESE_VARIABLES_H

#include <vector>
#include <string>
#include "comptime.h"

namespace cheese::curdle {
    struct ComptimeValue;
    struct Type;
    struct TopLevelVariableInfo {
        bool constant;
        bool pub; //Vi
        std::string mangled_name;
        Type *type;
    };

    struct ComptimeVariableInfo {
        bool pub;
        bool constant;
        Type *type;
        ComptimeValue *value;
    };

    struct RuntimeVariableInfo {
        bool constant;
        std::string runtime_name;
        Type *type;
    };

}
#endif //CHEESE_VARIABLES_H
