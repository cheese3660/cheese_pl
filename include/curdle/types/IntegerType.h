//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_INTEGERTYPE_H
#define CHEESE_INTEGERTYPE_H

#include "curdle/Type.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct IntegerType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bool sign{false};
        std::uint16_t size;

        static IntegerType *get(GlobalContext *gctx, bool sign, std::uint16_t size);

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~IntegerType() override = default;

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit = true) override;

        std::string to_string() override;

        memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gctx) override;

        memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

    private:

        IntegerType(bool sn, std::uint16_t sz) : sign(sn), size(sz) {}
    };
}

#endif //CHEESE_INTEGERTYPE_H
