//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_STRUCTURE_H
#define CHEESE_STRUCTURE_H

#include "Type.h"
#include <string>
#include <vector>
#include <map>
#include "parser/Node.h"
#include "variables.h"

namespace cheese::curdle {
    struct StructureField {
        std::string name;
        Type *type;
        bool pub; //Whether or not this is a public field
    };

    struct LazyValue {
        std::string name; // The name of the lazy value
        parser::NodePtr node; // The parser node that this corresponds to
    };

    struct Structure : Type {
        bacteria::TypePtr get_bacteria_type() override;

        void mark_references() override;

        ~Structure() override;

        // This contains all the information of a structure. which is a type
        // It does not have its interfaces attached to it as that is part of the main type
        std::vector<StructureField> fields;
        std::map<std::string, TopLevelVariableInfo> top_level_variables;
        std::vector<LazyValue> lazies;
    };
}

#endif //CHEESE_STRUCTURE_H
