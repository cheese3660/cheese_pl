//
// Created by Lexi Allen on 6/29/2023.
//

#ifndef CHEESE_COMPTIMECOMPLEXTYPE_H
#define CHEESE_COMPTIMECOMPLEXTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct ComptimeComplexType : Type {

        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~ComptimeComplexType() override = default;

        static ComptimeComplexType *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

    private:

        ComptimeComplexType() = default;
    };
}
#endif //CHEESE_COMPTIMECOMPLEXTYPE_H
