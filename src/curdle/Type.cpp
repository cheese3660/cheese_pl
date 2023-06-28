//
// Created by Lexi Allen on 4/1/2023.
//

#include "curdle/Type.h"
#include "curdle/comptime.h"
#include "error.h"
#include "curdle/curdle.h"
#include <typeinfo>
#include "curdle/GlobalContext.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/Float64Type.h"

namespace cheese::curdle {


    void Type::mark_references() {
        for (auto mixin: mixins) {
            mixin->mark();
        }
        mark_type_references();
    }

    bacteria::TypePtr Type::get_cached_type() {
        if (cached_type) {
            return cached_type;
        } else {
            cached_type = std::move(get_bacteria_type());
            return cached_type;
        }
    }


    std::string get_peer_type_list(std::vector<Type *> types) {
        std::stringstream ss{};
        for (int i = 0; i < types.size(); i++) {
            ss << types[i]->to_string();
            if (i < types.size() - 1) {
                ss << ", ";
            }
            if (i > 0 and i == types.size() - 2) {
                ss << "and ";
            }
        }
        return ss.str();
    }

    gcref<Type> peer_type(std::vector<Type *> types, GlobalContext *gctx) {
        if (types.empty()) {
            throw CurdleError{"No Peer Type: cannot find a peer type w/o any types to find a peer between",
                              error::ErrorCode::NoPeerType};
        }
        auto base_type = gcref{gctx->gc, types[0]};
        for (int i = 1; i < types.size(); i++) {
            if (types[i] != base_type)
                base_type = base_type->peer(types[i], gctx);
            if (base_type.value == nullptr) {
                throw CurdleError{
                        "No Peer Type: cannot find a peer type between " + get_peer_type_list(types),
                        error::ErrorCode::NoPeerType,
                };
            }
        }
        return base_type;
    }

    bool trivial_arithmetic_type(Type *type) {
#define TRIVIAL(T) if (dynamic_cast<T*>(type)) return true
        TRIVIAL(IntegerType);
        TRIVIAL(ComptimeIntegerType);
        TRIVIAL(Float64Type);
        TRIVIAL(ComptimeFloatType);
#undef TRIVIAL
        return false;
    }
}