//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/BooleanType.h"
#include "curdle/types/AnyType.h"
#include "project/GlobalContext.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/curdle.h"

namespace cheese::curdle {
    bacteria::TypePtr BooleanType::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::UnsignedInteger, 1);
    }

    void BooleanType::mark_type_references() {

    }

    BooleanType *BooleanType::get(cheese::project::GlobalContext *gctx) {
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

    gcref<Type> BooleanType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }

    gcref<ComptimeValue> BooleanType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        if (key == "__size__") {
            return gctx->gc.gcnew<ComptimeInteger>(get_cached_type()->get_llvm_size(gctx),
                                                   ComptimeIntegerType::get(gctx));
        }
        if (key == "__name__") {
            return gctx->gc.gcnew<ComptimeString>("bool");
        }
        if (key == "operator &") {
            // Generating the functions for operators and such, will be hell
        }
        throw CurdleError("key not a comptime child of type bool: " + key, error::ErrorCode::InvalidSubscript);
    }
}