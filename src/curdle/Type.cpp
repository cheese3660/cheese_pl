//
// Created by Lexi Allen on 4/1/2023.
//

#include "curdle/Type.h"
#include "curdle/comptime.h"
#include "error.h"
#include "curdle/curdle.h"
#include <typeinfo>
#include "project/GlobalContext.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/Float64Type.h"
#include "curdle/types/ComptimeComplexType.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/types/BooleanType.h"
#include "curdle/types/VoidType.h"
#include "curdle/types/ComposedFunctionType.h"
#include "curdle/types/FunctionPointerType.h"
#include "curdle/types/PointerType.h"
#include "curdle/types/ArrayType.h"

namespace cheese::curdle {


    void Type::mark_references() {
        for (auto mixin: mixins) {
            mixin->mark();
        }
        mark_type_references();
    }

    bacteria::TypePtr Type::get_cached_type(bacteria::nodes::BacteriaProgram *program) {
        if (cached_type) {
            return cached_type;
        } else {
            cached_type = std::move(get_bacteria_type(program));
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

    gcref<Type> peer_type(std::vector<Type *> types, cheese::project::GlobalContext *gctx) {
        if (types.empty()) {
            throw CurdleError{"No Peer Type: cannot find a peer type w/o any types to find a peer between",
                              error::ErrorCode::NoPeerType};
        }
        std::vector<gcref<Type>> _keepInScope;
//        auto base_type = gcref{gctx->gc, types[0]};
        auto base_type = types[0];
        _keepInScope.emplace_back(gctx->gc, base_type);
        for (int i = 1; i < types.size(); i++) {
            if (types[i] != base_type) {
                auto ref = base_type->peer(types[i], gctx);
                base_type = ref.value;
                _keepInScope.push_back(std::move(ref));
            }
            if (base_type == nullptr) {
                throw CurdleError{
                        "No Peer Type: cannot find a peer type between " + get_peer_type_list(types),
                        error::ErrorCode::NoPeerType,
                };
            }
        }
        return {gctx->gc, base_type};
    }

    bool trivial_arithmetic_type(Type *type) {
#define TRIVIAL(T) if (dynamic_cast<T*>(type)) return true
        TRIVIAL(IntegerType);
        TRIVIAL(ComptimeIntegerType);
        TRIVIAL(Float64Type);
        TRIVIAL(ComptimeFloatType);
        TRIVIAL(ComptimeComplexType);
        TRIVIAL(Complex64Type);
#undef TRIVIAL
        return false;
    }

    memory::garbage_collection::gcref<Type>
    binary_result_type(enums::SimpleOperation op, Type *a, Type *b, cheese::project::GlobalContext *gctx) {
        if (trivial_arithmetic_type(a) && trivial_arithmetic_type(b)) {
            switch (op) {
                case enums::SimpleOperation::Multiplication:
                case enums::SimpleOperation::Division:
                case enums::SimpleOperation::Remainder:
                case enums::SimpleOperation::Addition:
                case enums::SimpleOperation::Subtraction:
                case enums::SimpleOperation::LeftShift:
                case enums::SimpleOperation::RightShift:
                case enums::SimpleOperation::And:
                case enums::SimpleOperation::Or:
                case enums::SimpleOperation::Xor:
                    return peer_type({a, b}, gctx);
                case enums::SimpleOperation::LesserThan:
                case enums::SimpleOperation::GreaterThan:
                case enums::SimpleOperation::LesserThanOrEqualTo:
                case enums::SimpleOperation::GreaterThanOrEqualTo:
                case enums::SimpleOperation::EqualTo:
                case enums::SimpleOperation::NotEqualTo:
                    return {gctx->gc, BooleanType::get(gctx)};
            }
        }
        if (is_functional_type(a) || is_functional_type(b)) {
            return {gctx->gc, ComposedFunctionType::get(gctx, op, {a, b})};
        }


        return {gctx->gc, VoidType::get(gctx)};
    }

    memory::garbage_collection::gcref<Type>
    unary_result_type(enums::SimpleOperation op, Type *t, cheese::project::GlobalContext *gctx) {
        if (trivial_arithmetic_type(t)) {
            return {gctx->gc, t};
        } else if (is_functional_type(t)) {
            return {gctx->gc, ComposedFunctionType::get(gctx, op, {t})};
        }
        return {gctx->gc, VoidType::get(gctx)};
    }

    bool is_functional_type(Type *type) {
#define FUNCTIONAL(T) if (dynamic_cast<T*>(type)) return true
        FUNCTIONAL(FunctionPointerType);
        FUNCTIONAL(ComposedFunctionType);
#undef FUNCTIONAL
        return false;
    }

    memory::garbage_collection::gcref<Type>
    get_functional_return_type(Type *type, cheese::project::GlobalContext *gctx) {
#define WHEN_FUNCTIONAL_IS(T, name) if (auto name = dynamic_cast<T*>(type); name)
        WHEN_FUNCTIONAL_IS(FunctionPointerType, pFunctionPointerType) {
            return {gctx->gc, pFunctionPointerType->return_type};
        }
        WHEN_FUNCTIONAL_IS(ComposedFunctionType, pComposedFunctionType) {
            return pComposedFunctionType->get_return_type(gctx);
        }
        NOT_IMPL_FOR(typeid(*type).name());
    }

    std::vector<memory::garbage_collection::gcref<Type>>
    get_functional_argument_types(Type *type, cheese::project::GlobalContext *gctx) {
        WHEN_FUNCTIONAL_IS(FunctionPointerType, pFunctionPointerType) {
            std::vector<gcref<Type>> result = {};
            for (const auto &ty: pFunctionPointerType->argument_types) {
                result.emplace_back(gctx->gc, ty);
            }
            return result;
        }
        WHEN_FUNCTIONAL_IS(ComposedFunctionType, pComposedFunctionType) {
            return pComposedFunctionType->get_argument_types(gctx);
        }
#undef WHEN_FUNCTIONAL_IS
        NOT_IMPL_FOR(typeid(*type).name());
    }

    gcref<Type> get_true_subtype(garbage_collector &gc, Type *type, std::size_t num_subindices) {
        if (num_subindices == 0) return {gc, type};
#define WHEN_TY_IS(ty, name) if (auto name = dynamic_cast<ty*>(type); name)
        WHEN_TY_IS(PointerType, pPointerType) {
            return get_true_subtype(gc, type, num_subindices - 1);
        }
        WHEN_TY_IS(ArrayType, pArrayType) {
            if (num_subindices >= pArrayType->dimensions.size()) {
                return get_true_subtype(gc, pArrayType->subtype, num_subindices - pArrayType->dimensions.size());
            }
            if (num_subindices < pArrayType->dimensions.size()) {
                auto num_ptrs = num_subindices - pArrayType->dimensions.size();
                auto base_ptr = gc.gcnew<PointerType>(pArrayType->subtype, pArrayType->constant);
                for (int i = 1; i < num_ptrs; i++) {
                    base_ptr = gc.gcnew<PointerType>(base_ptr, pArrayType->constant);
                }
                return base_ptr;
            }
        }
#undef WHEN_TY_IS
        throw CurdleError{
                "Cannot get subtype of: " + std::string(typeid(*type).name()),
                error::ErrorCode::InvalidSubscript
        };
    }

}