//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/NoReturnType.h"
#include "project/GlobalContext.h"
#include "curdle/curdle.h"

namespace cheese::curdle {
    NoReturnType *NoReturnType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: noreturn")) {
            auto ref = gctx->gc.gcnew<NoReturnType>();
            gctx->cached_objects["type: noreturn"] = ref;
            return ref;
        }
        return dynamic_cast<NoReturnType *>(gctx->cached_objects["type: noreturn"]);
    }

    bacteria::TypePtr NoReturnType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        // Wait this might be an error
        NO_BACTERIA_TYPE;
    }

    void NoReturnType::mark_type_references() {

    }

    Comptimeness NoReturnType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t NoReturnType::compare(Type *other, bool implicit) {
        return 0; // No return can "cast" to anything
    }

    std::string NoReturnType::to_string() {
        return "noreturn";
    }

    gcref<Type> NoReturnType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        return REF(other);
    }
}