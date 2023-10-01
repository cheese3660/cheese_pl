//
// Created by Lexi Allen on 9/30/2023.
//

#ifndef CHEESE_COMPOSEDFUNCTIONTYPE_H
#define CHEESE_COMPOSEDFUNCTIONTYPE_H

#include "curdle/Type.h"
#include "curdle/values/ComptimeType.h"
#include "project/GlobalContext.h"
#include "curdle/enums//SimpleOperation.h"

namespace cheese::curdle {
    struct ComposedFunctionType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        static ComposedFunctionType *
        get(GlobalContext *gctx, enums::SimpleOperation operation, const std::vector<Type *> &operand_types);

        bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) override;

        void mark_type_references() override;

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other, bool implicit) override;

        gcref<Type> peer(Type *other, cheese::project::GlobalContext *gctx) override;

        gcref<ComptimeValue> get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) override;

        std::string to_string() override;

        ~ComposedFunctionType() override = default;

        enums::SimpleOperation operation;
        std::vector<Type *> operand_types; //a nullptr anywhere in here means to get the next reference from the comptime values list

        gcref<Type> get_return_type(cheese::project::GlobalContext *gctx);

        std::vector<gcref<Type>>
        get_argument_types(cheese::project::GlobalContext *gctx); // this is the argument types sans state

        std::string get_function_name(
                cheese::project::ComptimeContext *cctx); // This will generate the concrete function if it isn't generated already

    private:
        std::string cached_function_name;

        ComposedFunctionType(enums::SimpleOperation operation, std::vector<Type *> operand_types);
    };
}

#endif //CHEESE_COMPOSEDFUNCTIONTYPE_H
