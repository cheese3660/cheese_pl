//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/VoidType.h"
#include "project/GlobalContext.h"
#include "curdle/curdle.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/types/IntegerType.h"
#include "curdle/values/ComptimeInteger.h"

namespace cheese::curdle {
    bacteria::TypePtr VoidType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        return program->get_type(bacteria::BacteriaType::Type::Void);
    }

    void VoidType::mark_type_references() {

    }

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

    memory::garbage_collection::gcref<ComptimeValue>
    VoidType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        INVALID_CHILD;
    }
}