//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/FunctionTemplateType.h"
#include "curdle/types/AnyType.h"
#include "project/GlobalContext.h"

namespace cheese::curdle {
    bacteria::TypePtr FunctionTemplateType::get_bacteria_type() {
        return {};
    }

    void FunctionTemplateType::mark_type_references() {
    }


    FunctionTemplateType *FunctionTemplateType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: function_template")) {
            auto ref = gctx->gc.gcnew<FunctionTemplateType>();
            gctx->cached_objects["type: function_template"] = ref;
            return ref;
        }
        return dynamic_cast<FunctionTemplateType *>(gctx->cached_objects["type: function_template"]);
    }

    Comptimeness FunctionTemplateType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t FunctionTemplateType::compare(Type *other, bool implicit) {
        return other == this ? 0 : -1;
    }

    std::string FunctionTemplateType::to_string() {
        return "$FunctionSet";
    }

    gcref<Type> FunctionTemplateType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return REF(other);
    }

    memory::garbage_collection::gcref<ComptimeValue>
    FunctionTemplateType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        INVALID_CHILD;
    }
}