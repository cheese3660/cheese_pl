//
// Created by Lexi Allen on 9/30/2023.
//
#include "curdle/types/ComptimeEnumType.h"
#include "project/GlobalContext.h"
#include "curdle/curdle.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/types/IntegerType.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/AnyType.h"

namespace cheese::curdle {
    bacteria::TypePtr ComptimeEnumType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        NO_BACTERIA_TYPE;
    }

    void ComptimeEnumType::mark_type_references() {

    }

    ComptimeEnumType *ComptimeEnumType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: $ComptimeEnum")) {
            auto ref = gctx->gc.gcnew<ComptimeEnumType>();
            gctx->cached_objects["type: $ComptimeEnum"] = ref;
            return ref;
        }
        return dynamic_cast<ComptimeEnumType *>(gctx->cached_objects["type: $ComptimeEnum"]);
    }

    Comptimeness ComptimeEnumType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ComptimeEnumType::compare(Type *other, bool implicit) {
        return other == this ? 0 : -1;
        // TODO: Enum types
    }

    std::string ComptimeEnumType::to_string() {
        return "$ComptimeEnum";
    }

    gcref<Type> ComptimeEnumType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        // TODO: Enum types
        NO_PEER;
    }

    memory::garbage_collection::gcref<ComptimeValue>
    ComptimeEnumType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        INVALID_CHILD;
    }
}