//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_IMPORTEDFUNCTIONTYPE_H
#define CHEESE_IMPORTEDFUNCTIONTYPE_H

// This defines the type of an imported function

#include "curdle/Type.h"
#include "project/GlobalContext.h"

namespace cheese::curdle {
    struct ImportedFunctionType : Type {
        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit) override;

        memory::garbage_collection::gcref<Type> peer(Type *other, cheese::project::GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

        std::string to_string() override;

        ~ImportedFunctionType() override = default;

        Type *return_type;
        std::vector<Type *> argument_types;

        ImportedFunctionType(Type *returnType, std::vector<Type *> argument_types);
    };
}

#endif //CHEESE_IMPORTEDFUNCTIONTYPE_H
