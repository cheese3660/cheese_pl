//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/VoidType.h"
#include "project/GlobalContext.h"
#include "GlobalContext.h"

namespace cheese::curdle {
    bacteria::TypePtr VoidType::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Void);
    }

    void VoidType::mark_type_references() {

    }

    static VoidType *vt_instance;

    VoidType *VoidType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: void")) {
            auto ref = gctx->gc.gcnew<VoidType>();
            gctx->cached_objects["type: void"] = ref;
            return ref;
        }
        return dynamic_cast<VoidType *>(gctx->cached_objects["type: void"]);
    }

    Comptimeness VoidType::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t VoidType::compare(Type *other, bool implicit) {
        return other == this ? 0 : -1;
    }

    std::string VoidType::to_string() {
        return "void";
    }

    gcref<Type> VoidType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        return REF(this);
    }
}