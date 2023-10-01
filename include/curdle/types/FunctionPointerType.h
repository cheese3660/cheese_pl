//
// Created by Lexi Allen on 9/29/2023.
//

#ifndef CHEESE_FUNCTIONPOINTERTYPE_H
#define CHEESE_FUNCTIONPOINTERTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

namespace cheese::curdle {
    struct FunctionPointerType : Type {
        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit) override;

        memory::garbage_collection::gcref<Type> peer(Type *other, cheese::project::GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

        std::string to_string() override;

        ~FunctionPointerType() override = default;

        Type *return_type;
        std::vector<Type *> argument_types;
    };
}

#endif //CHEESE_FUNCTIONPOINTERTYPE_H
