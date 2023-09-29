//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMEINTEGERTYPE_H
#define CHEESE_COMPTIMEINTEGERTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct ComptimeIntegerType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        ~ComptimeIntegerType() override = default;

        static ComptimeIntegerType *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

    private:

        ComptimeIntegerType() = default;
    };
}
#endif //CHEESE_COMPTIMEINTEGERTYPE_H
