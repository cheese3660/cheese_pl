//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_TYPE_H
#define CHEESE_TYPE_H

#include "bacteria/BacteriaType.h"
#include "Mixin.h"
#include "memory/garbage_collection.h"
#include "project/GlobalContext.h"
#include "bacteria/nodes/receiver_nodes.h"
#include "curdle/enums/SimpleOperation.h"

namespace cheese::curdle {
#define PEER_TYPE_CATCH_ANY() if (dynamic_cast<AnyType*>(other)) return gcref{gctx->gc,this}
#define NO_PEER gcref<Type>{gctx->gc,nullptr}
#define REF(X) gcref{gctx->gc,X}
#define NO_BACTERIA_TYPE throw cheese::curdle::CurdleError{"No bacteria type for " + to_string(), error::ErrorCode::NoBacteriaType}
    struct ComptimeValue;
    enum class Comptimeness {
        Comptime,
        ArgumentDepending, //For "any" types
        Runtime
    };

    struct Type : memory::garbage_collection::managed_object {
        // This function is used for converting a type into bacteria.
        bacteria::TypePtr get_cached_type(bacteria::nodes::BacteriaProgram *program);

        virtual bacteria::TypePtr get_bacteria_type(bacteria::nodes::BacteriaProgram *program) = 0;

        virtual void mark_type_references() = 0;

        virtual Comptimeness get_comptimeness() = 0;

        // This should be done in a way that the other type is *always* concrete and not an "any"
        // So basically, its comparison in terms of casting *to*
        virtual std::int32_t compare(Type *other, bool implicit = true) = 0;

        virtual memory::garbage_collection::gcref<Type> peer(Type *other, cheese::project::GlobalContext *gctx) = 0;

        virtual memory::garbage_collection::gcref<ComptimeValue>
        get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) = 0;

        virtual std::string to_string() = 0;

        // This is where we add mixins and such

        std::vector<Mixin *> mixins;
        // So we don't have to do funky stuff :3
        bacteria::TypePtr cached_type = nullptr;

        void mark_references() final;


        ~Type() override = default;
    };


    memory::garbage_collection::gcref<Type> peer_type(std::vector<Type *> types, cheese::project::GlobalContext *gctx);

    memory::garbage_collection::gcref<Type>
    binary_result_type(enums::SimpleOperation op, Type *a, Type *b, cheese::project::GlobalContext *gctx);

    memory::garbage_collection::gcref<Type>
    unary_result_type(enums::SimpleOperation op, Type *t, cheese::project::GlobalContext *gctx);

    bool trivial_arithmetic_type(Type *type);

    bool is_functional_type(Type *type);

    memory::garbage_collection::gcref<Type>
    get_functional_return_type(Type *type, cheese::project::GlobalContext *gctx);

    std::vector<memory::garbage_collection::gcref<Type>>
    get_functional_argument_types(Type *type, cheese::project::GlobalContext *gctx);

    memory::garbage_collection::gcref<Type>
    get_true_subtype(memory::garbage_collection::garbage_collector &gc, Type *type, std::size_t num_subindices);
}
#define INVALID_CHILD throw CurdleError("key not a comptime child of type " + to_string() + ": " + key, error::ErrorCode::InvalidSubscript)
#define CATCH_DUNDER_NAME do { if (key == "__name__") { return gctx->gc.gcnew<ComptimeString>(to_string(), ComptimeStringType::get(gctx)); } } while(0)
#define CATCH_DUNDER_SIZE do { if (key == "__size__") { return gctx->gc.gcnew<ComptimeInteger>(get_cached_type(gctx->global_receiver.get())->get_llvm_size(gctx), IntegerType::get(gctx, false, gctx->machine.data_pointer_size));}} while (0)
#endif //CHEESE_TYPE_H
