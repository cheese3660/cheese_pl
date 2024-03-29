//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_STRUCTURE_H
#define CHEESE_STRUCTURE_H

#include <string>
#include <vector>
#include <map>
#include "parser/node.h"
#include "curdle/Interface.h"
#include "curdle/variables.h"
#include "curdle/functions.h"
#include "curdle/comptime.h"
#include "project/GlobalContext.h"
#include "curdle/Type.h"

using namespace cheese::project;

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
        Structure(std::string name, ComptimeContext *ctx, memory::garbage_collection::garbage_collector &gc);

        ComptimeContext *containedContext;

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        ~Structure() override;

        // This contains all the information of a structure. which is a type
        // It does not have its interfaces attached to it as that is part of the main type
        // If the type of the structure is a tuple, then the names of the fields will be 0, 1, 2, 3, 4, 5, etc...
        bool is_tuple{false};
        bool implicit_type{false}; // If it is an implicit type, then it should compare to any other type
        std::vector<StructureField> fields;
        std::map<std::string, TopLevelVariableInfo> top_level_variables;
        std::map<std::string, ComptimeVariableInfo> comptime_variables;
        std::vector<LazyValue *> lazies;
        std::vector<Interface *> interfaces; // Separate from mixins as it isn't defining functions outside the structure
        std::map<std::string, FunctionSet *> function_sets;
        std::string name; // Structures must have names bound to them, at some point, unbound names start with ::(counter) which places where names can be bound automatically bind it by matching for a structure name starting with "::"
        void resolve_lazy(LazyValue *&lazy);

        void resolve_by_name(const std::string &name);


        void search_entry();

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gc) override;

        gcref<ComptimeValue> get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

    };
}

#endif //CHEESE_STRUCTURE_H
