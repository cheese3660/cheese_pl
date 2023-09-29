//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_VOIDTYPE_H
#define CHEESE_VOIDTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct VoidType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        ~VoidType() override = default;

        static VoidType *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

    private:

        VoidType() = default;
    };
}

#endif //CHEESE_VOIDTYPE_H
