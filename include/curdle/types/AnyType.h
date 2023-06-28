//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_ANYTYPE_H
#define CHEESE_ANYTYPE_H

#include "curdle/Type.h"

namespace cheese::curdle {

    struct AnyType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~AnyType() override = default;

        static AnyType *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

    private:

        AnyType() = default;
    };
}
#endif //CHEESE_ANYTYPE_H
