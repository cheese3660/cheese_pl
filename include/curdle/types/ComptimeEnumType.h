//
// Created by Lexi Allen on 9/30/2023.
//

#ifndef CHEESE_COMPTIMEENUMTYPE_H
#define CHEESE_COMPTIMEENUMTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct ComptimeEnumType : Type {

        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        ~ComptimeEnumType() override = default;

        static ComptimeEnumType *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

    private:

        ComptimeEnumType() = default;
    };
}
#endif //CHEESE_COMPTIMEENUMTYPE_H
