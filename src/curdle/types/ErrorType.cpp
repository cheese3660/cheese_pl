//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/ErrorType.h"
#include "curdle/types/AnyType.h"
#include "curdle/GlobalContext.h"
#include "curdle/curdle.h"

namespace cheese::curdle {
    bacteria::TypePtr ErrorType::get_bacteria_type() {
        NO_BACTERIA_TYPE(error_type);
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
    gcref<Type> ErrorType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }

    std::string ErrorType::to_string() {
        return "ERROR!";
    }


    ErrorType *ErrorType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: error")) {
            auto ref = gctx->gc.gcnew<ErrorType>();
            gctx->cached_objects["type: error"] = ref;
            return ref;
        }
        return dynamic_cast<ErrorType *>(gctx->cached_objects["type: error"]);
    }
}