//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/BooleanType.h"
#include "curdle/types/AnyType.h"
#include "curdle/GlobalContext.h"

namespace cheese::curdle {
    bacteria::TypePtr BooleanType::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::UnsignedInteger, 1);
    }

    void BooleanType::mark_type_references() {

    }

    BooleanType *BooleanType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: boolean")) {
            auto ref = gctx->gc.gcnew<BooleanType>();
            gctx->cached_objects["type: boolean"] = ref;
            return ref;
        }
        return dynamic_cast<BooleanType *>(gctx->cached_objects["type: boolean"]);
    }

    Comptimeness BooleanType::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t BooleanType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        return -1;
    }

    std::string BooleanType::to_string() {
        return "bool";
    }

    gcref<Type> BooleanType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }
}