//
// Created by Lexi Allen on 6/28/2023.
//

#include "curdle/types/TypeType.h"
#include "project/GlobalContext.h"
#include "curdle/curdle.h"
#include "curdle/types/AnyType.h"
#include "GlobalContext.h"

namespace cheese::curdle {
    bacteria::TypePtr TypeType::get_bacteria_type() {
        NO_BACTERIA_TYPE(type);
    }

    void TypeType::mark_type_references() {

    }


    TypeType *TypeType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: type")) {
            auto ref = gctx->gc.gcnew<TypeType>();
            gctx->cached_objects["type: type"] = ref;
            return ref;
        }
        return dynamic_cast<TypeType *>(gctx->cached_objects["type: type"]);
    }

    Comptimeness TypeType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t TypeType::compare(Type *other, bool implicit) {
        return (other == this) ? 0 : -1; // Since this is a singleton, this should be all that is necessary
    }

    std::string TypeType::to_string() {
        return "type";
    }

    gcref<Type> TypeType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }
}