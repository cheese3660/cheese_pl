//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_VARIABLES_H
#define CHEESE_VARIABLES_H

#include <vector>
#include <string>
#include "comptime.h"
#include "Type.h"

namespace cheese::curdle {
    struct TopLevelVariableInfo {
        bool exported;
        bool pub; //Vi
        std::string mangled_name;
        Type *type;
    };

    struct ComptimeVariableInfo {
        bool pub;
        Type *type;
        ComptimeValue *value;
    };
}
#endif //CHEESE_VARIABLES_H
