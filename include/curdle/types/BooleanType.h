//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_BOOLEANTYPE_H
#define CHEESE_BOOLEANTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct BooleanType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~BooleanType() override = default;

        static BooleanType *get(cheese::project::GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, GlobalContext *gctx) override;

    private:

        BooleanType() = default;
    };
}
#endif //CHEESE_BOOLEANTYPE_H
