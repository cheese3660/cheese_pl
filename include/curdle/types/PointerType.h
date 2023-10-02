//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_POINTERTYPE_H
#define CHEESE_POINTERTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;
namespace cheese::curdle {
    struct PointerType : Type {
        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gc) override;

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        PointerType(Type *subtype, bool constant);

        ~PointerType() override = default;


        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, cheese::project::GlobalContext *gctx) override;

        Type *subtype;
        bool constant;
    };
}
#endif //CHEESE_POINTERTYPE_H
