//
// Created by Lexi Allen on 6/29/2023.
//

#ifndef CHEESE_COMPLEX64TYPE_H
#define CHEESE_COMPLEX64TYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct Complex64Type : Type {

        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        ~Complex64Type() override = default;

        static Complex64Type *get(GlobalContext *gctx);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, GlobalContext *gctx) override;

    private:

        Complex64Type() = default;
    };
}
#endif //CHEESE_COMPLEX64TYPE_H
