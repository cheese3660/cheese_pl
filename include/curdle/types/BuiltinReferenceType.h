//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_BUILTINREFERENCETYPE_H
#define CHEESE_BUILTINREFERENCETYPE_H

#include "curdle/Type.h"

namespace cheese::curdle {
    struct BuiltinReferenceType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~BuiltinReferenceType() override = default;

        static BuiltinReferenceType *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

    private:
        BuiltinReferenceType() = default;
    };
}

#endif //CHEESE_BUILTINREFERENCETYPE_H
