//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/BuiltinReferenceType.h"
#include "curdle/types/AnyType.h"
#include "curdle/curdle.h"
#include "curdle/GlobalContext.h"

namespace cheese::curdle {
    bacteria::TypePtr BuiltinReferenceType::get_bacteria_type() {
        NO_BACTERIA_TYPE(builtin_reference);
    }

    void BuiltinReferenceType::mark_type_references() {

    }

    BuiltinReferenceType *BuiltinReferenceType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: builtin_reference")) {
            auto ref = gctx->gc.gcnew<BuiltinReferenceType>();
            gctx->cached_objects["type: builtin_reference"] = ref;
            return ref;
        }
        return dynamic_cast<BuiltinReferenceType *>(gctx->cached_objects["type: builtin_reference"]);
    }


    Comptimeness BuiltinReferenceType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t BuiltinReferenceType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        return -1;
    }

    std::string BuiltinReferenceType::to_string() {
        return "$BuiltinReference";
    }

    gcref<Type> BuiltinReferenceType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }
}