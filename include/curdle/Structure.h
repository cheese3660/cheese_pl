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
#include "Interface.h"
#include "variables.h"
#include "functions.h"
#include "comptime.h"

namespace cheese::curdle {
    struct FunctionSet;
    struct ComptimeContext;
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
        ComptimeContext *containedContext;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~Structure() override;

        // This contains all the information of a structure. which is a type
        // It does not have its interfaces attached to it as that is part of the main type
        // If the type of the structure is a tuple, then the names of the fields will be _0, _1, _2, _3, _4, _5, etc...
        bool is_tuple{false};
        std::vector<StructureField> fields;
        std::map<std::string, TopLevelVariableInfo> top_level_variables;
        std::map<std::string, ComptimeVariableInfo> comptime_variables;
        std::vector<LazyValue *> lazies;
        std::vector<Interface *> interfaces; // Separate from mixins as it isn't defining functions outside the structure
        std::map<std::string, FunctionSet *> function_sets;
        std::string name; // Structures must have names bound to them

        void resolve_lazy(LazyValue *&lazy);

        void resolve_by_name(const std::string &name);


        void search_entry();

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;
    };
}

#endif //CHEESE_STRUCTURE_H
