//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/ReferenceType.h"
#include "project/GlobalContext.h"
#include "curdle/types/AnyType.h"

namespace cheese::curdle {
    bacteria::TypePtr ReferenceType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        auto child_type = child->get_cached_type(program);
        return program->get_type(bacteria::BacteriaType::Type::Reference, {}, child_type);
    }

    void ReferenceType::mark_type_references() {
        child->mark();
    }

    Comptimeness ReferenceType::get_comptimeness() {
        return child->get_comptimeness();
    }

    // This kinda casting has to be specifically done w/ a "possible rvalue address" in bacteria which is &&(value)
    // Rather than a regular address of operator which is &value
    // But on a known lvalue the possible rvalue address gets converted into an lvalue in second stage lowering
    int32_t ReferenceType::compare(Type *other, bool implicit) {
        if (auto other_r = dynamic_cast<ReferenceType *>(other); other_r && child->compare(other_r->child) == 0) {
            return 0;
        }
        auto child_comparison = child->compare(other);
        if (child_comparison != -1) {
            return child_comparison + 1;
        }
        return -1;
    }

    std::string ReferenceType::to_string() {
        return (constant ? "*~" : "*") + child->to_string();
    }

    gcref<Type> ReferenceType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (compare(other) == 0) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }
}