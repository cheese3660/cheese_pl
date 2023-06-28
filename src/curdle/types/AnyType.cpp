//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/AnyType.h"
#include "curdle/GlobalContext.h"
#include "curdle/curdle.h"

namespace cheese::curdle {
    bacteria::TypePtr AnyType::get_bacteria_type() {
        NO_BACTERIA_TYPE(any);
    }

    void AnyType::mark_type_references() {

    }


    AnyType *AnyType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: any")) {
            auto ref = gctx->gc.gcnew<AnyType>();
            gctx->cached_objects["type: any"] = ref;
            return ref;
        }
        return dynamic_cast<AnyType *>(gctx->cached_objects["type: any"]);
    }

    Comptimeness AnyType::get_comptimeness() {
        return Comptimeness::ArgumentDepending;
    }

    int32_t AnyType::compare(Type *other, bool implicit) {
        return 131072;
    }

    std::string AnyType::to_string() {
        return "any";
    }

    gcref<Type> AnyType::peer(Type *other, GlobalContext *gctx) {
        return REF(other);
    }
}