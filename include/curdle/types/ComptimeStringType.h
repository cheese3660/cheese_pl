//
// Created by Lexi Allen on 9/29/2023.
//

#ifndef CHEESE_COMPTIMESTRINGTYPE_H
#define CHEESE_COMPTIMESTRINGTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

namespace cheese::curdle {
    using namespace cheese::project;

    struct ComptimeStringType : Type {

        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        ~ComptimeStringType() override = default;

        static ComptimeStringType *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

    private:

        ComptimeStringType() = default;
    };
}
#endif //CHEESE_COMPTIMESTRINGTYPE_H
