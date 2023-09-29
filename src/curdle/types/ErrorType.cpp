//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/ErrorType.h"
#include "curdle/types/AnyType.h"
#include "project/GlobalContext.h"
#include "curdle/curdle.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"

namespace cheese::curdle {
    bacteria::TypePtr ErrorType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        NO_BACTERIA_TYPE;
    }

    void ErrorType::mark_type_references() {

    }

    Comptimeness ErrorType::get_comptimeness() {
        return Comptimeness::Comptime;
    }


    int32_t ErrorType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        return -1;
    }

    // Lets always coalesce error types w/ peer types
    gcref<Type> ErrorType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }

    std::string ErrorType::to_string() {
        return "ERROR!";
    }


    ErrorType *ErrorType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: error")) {
            auto ref = gctx->gc.gcnew<ErrorType>();
            gctx->cached_objects["type: error"] = ref;
            return ref;
        }
        return dynamic_cast<ErrorType *>(gctx->cached_objects["type: error"]);
    }

    gcref<ComptimeValue> ErrorType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        INVALID_CHILD;
    }
}