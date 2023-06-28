//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_TYPE_H
#define CHEESE_TYPE_H

#include "bacteria/BacteriaType.h"
#include "Mixin.h"
#include "memory/garbage_collection.h"


namespace cheese::curdle {
#define PEER_TYPE_CATCH_ANY() if (dynamic_cast<AnyType*>(other)) return gcref{gctx->gc,this}
#define NO_PEER gcref<Type>{gctx->gc,nullptr}
#define REF(X) gcref{gctx->gc,X}
#define NO_BACTERIA_TYPE(name) throw cheese::curdle::CurdleError{"No bacteria type for " # name, error::ErrorCode::NoBacteriaType};
    struct GlobalContext;
    enum class Comptimeness {
        Comptime,
        ArgumentDepending, //For "any" types
        Runtime
    };

    struct Type : memory::garbage_collection::managed_object {
        // This function is used for converting a type into bacteria.
        bacteria::TypePtr get_cached_type();

        virtual bacteria::TypePtr get_bacteria_type() = 0;

        virtual void mark_type_references() = 0;

        virtual Comptimeness get_comptimeness() = 0;

        // This should be done in a way that the other type is *always* concrete and not an "any"
        virtual std::int32_t compare(Type *other, bool implicit = true) = 0;

        virtual memory::garbage_collection::gcref<Type> peer(Type *other, GlobalContext *gc) = 0;

        virtual std::string to_string() = 0;

        // This is where we add mixins and such

        std::vector<Mixin *> mixins;
        // So we don't have to do funky stuff :3
        bacteria::TypePtr cached_type;

        void mark_references() final;


        ~Type() override = default;
    };


    memory::garbage_collection::gcref<Type> peer_type(std::vector<Type *> types, GlobalContext *gctx);

    bool trivial_arithmetic_type(Type *type);
}
#endif //CHEESE_TYPE_H
