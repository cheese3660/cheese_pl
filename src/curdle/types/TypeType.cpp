//
// Created by Lexi Allen on 6/28/2023.
//

#include "curdle/types/TypeType.h"
#include "project/GlobalContext.h"
#include "curdle/curdle.h"
#include "curdle/types/AnyType.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"

namespace cheese::curdle {
    bacteria::TypePtr TypeType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        NO_BACTERIA_TYPE;
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

    gcref<ComptimeValue> TypeType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        INVALID_CHILD;
    }
}