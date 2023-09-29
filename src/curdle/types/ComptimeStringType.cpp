//
// Created by Lexi Allen on 9/29/2023.
//
#include "curdle/types/ComptimeStringType.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/IntegerType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/curdle.h"
#include "curdle/types/AnyType.h"

namespace cheese::curdle {

    bacteria::TypePtr ComptimeStringType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        NO_BACTERIA_TYPE;
    }

    void ComptimeStringType::mark_type_references() {

    }

    ComptimeStringType *ComptimeStringType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: comptime_string")) {
            auto ref = gctx->gc.gcnew<ComptimeStringType>();
            gctx->cached_objects["type: comptime_string"] = ref;
            return ref;
        }
        return dynamic_cast<ComptimeStringType *>(gctx->cached_objects["type: comptime_string"]);
    }

    Comptimeness ComptimeStringType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ComptimeStringType::compare(Type *other, bool implicit) {
        // Implicitness doesn't matter here
        if (other == this) return 0;
        // Return -1 for now, as there are no array or slice types just yet
        return -1;
    }

    std::string ComptimeStringType::to_string() {
        return "comptime_string";
    }

    memory::garbage_collection::gcref<Type> ComptimeStringType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        NO_PEER;
    }

    memory::garbage_collection::gcref<ComptimeValue>
    ComptimeStringType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        INVALID_CHILD;
    }
}