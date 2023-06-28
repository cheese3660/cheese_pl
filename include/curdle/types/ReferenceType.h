//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_REFERENCETYPE_H
#define CHEESE_REFERENCETYPE_H

#include "curdle/Type.h"

namespace cheese::curdle {
    struct ReferenceType : Type {
        ReferenceType(Type *child, bool constant) : child(child), constant(constant) {}

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~ReferenceType() override = default;

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

        Type *child;
        bool constant; //Whether this is a constant reference type
    };
}

#endif //CHEESE_REFERENCETYPE_H
