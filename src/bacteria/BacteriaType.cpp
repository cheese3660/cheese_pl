//
// Created by Lexi Allen on 4/23/2023.
//
#include "bacteria/BacteriaType.h"
#include <sstream>

namespace cheese::bacteria {

    std::string bacteria::BacteriaType::to_string() {
        switch (type) {
            case Type::Opaque:
                return "opaque";
            case Type::Void:
                return "void";
            case Type::Noreturn:
                return "noreturn";
            case Type::UnsignedInteger:
                return "u" + std::to_string(integer_size);
            case Type::SignedInteger:
                return "i" + std::to_string(integer_size);
            case Type::Float32:
                return "f32";
            case Type::Float64:
                return "f64";
            case Type::Complex32:
                return "c32";
            case Type::Complex64:
                return "c64";
            case Type::Slice:
                return "<>" + subtype->to_string();
            case Type::Array: {
                std::stringstream ss{};
                ss << "[";
                for (int i = 0; i < array_dimensions.size(); i++) {
                    ss << array_dimensions[i];
                    if (i < array_dimensions.size() - 1) {
                        ss << ",";
                    }
                }
                ss << "]" << subtype->to_string();
                return ss.str();
            }
            case Type::Reference:
                return "*" + subtype->to_string();
            case Type::Pointer:
                return "[*]" + subtype->to_string();
            case Type::Object: {
                std::stringstream ss{};
                ss << "{";
                for (int i = 0; i < child_types.size(); i++) {
                    ss << child_types[i]->to_string();
                    if (i < child_types.size() - 1) {
                        ss << ",";
                    }
                }
                ss << "}";
                return ss.str();
            }
            case Type::UnsignedSize:
                return "usize";
            case Type::SignedSize:
                return "isize";
            case Type::WeakReference:
                return "*cyclic";
            case Type::WeakPointer:
                return "[*]cyclic";
            case Type::WeakSlice:
                return "<>cyclic";
        }
    }

    BacteriaType::BacteriaType(BacteriaType::Type type, uint16_t integerSize,
                               const std::shared_ptr<BacteriaType> &subtype,
                               const std::vector<std::size_t> &arrayDimensions,
                               const std::vector<std::shared_ptr<BacteriaType>> &childTypes,
                               const std::weak_ptr<BacteriaType> &weak_reference) : type(type), integer_size(
            integerSize), subtype(subtype), array_dimensions(arrayDimensions), child_types(childTypes),
                                                                                    weak_reference(weak_reference) {}

    llvm::Type *BacteriaType::get_llvm_type(cheese::project::GlobalContext *ctx) {
        if (cached_llvm_type) return cached_llvm_type;
        switch (type) {
            case Type::Opaque:
            case Type::Void:
            case Type::Noreturn:
                cached_llvm_type = llvm::Type::getVoidTy(ctx->llvm_context);
                break;
            case Type::UnsignedInteger:
            case Type::SignedInteger:
                cached_llvm_type = llvm::IntegerType::get(ctx->llvm_context, integer_size);
                break;
            case Type::UnsignedSize:
            case Type::SignedSize:
                cached_llvm_type = llvm::IntegerType::get(ctx->llvm_context, ctx->machine.data_pointer_size * 8);
                break;
            case Type::Float32:
                cached_llvm_type = llvm::Type::getFloatTy(ctx->llvm_context);
                break;
            case Type::Float64:
                cached_llvm_type = llvm::Type::getDoubleTy(ctx->llvm_context);
                break;
            case Type::Complex32: {
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                cached_llvm_type = struct_type;
                struct_type->setBody(llvm::ArrayRef<llvm::Type *>{llvm::Type::getFloatTy(ctx->llvm_context),
                                                                  llvm::Type::getFloatTy(ctx->llvm_context)}, true);
                break;
            }
            case Type::Complex64: {
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                cached_llvm_type = struct_type;
                struct_type->setBody(llvm::ArrayRef<llvm::Type *>{llvm::Type::getDoubleTy(ctx->llvm_context),
                                                                  llvm::Type::getDoubleTy(ctx->llvm_context)}, true);
                break;
            }
            case Type::Slice: {
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                cached_llvm_type = struct_type;
                struct_type->setBody(llvm::IntegerType::get(ctx->llvm_context, ctx->machine.data_pointer_size * 8),
                                     llvm::PointerType::get(subtype->get_llvm_type(ctx),
                                                            ctx->machine.data_pointer_addr));
                break;
            }
            case Type::Array:
                // Here we can flatten the array, and use math to index it normally when compiling
                cached_llvm_type = llvm::ArrayType::get(subtype->get_llvm_type(ctx),
                                                        std::accumulate(array_dimensions.begin(),
                                                                        array_dimensions.end(), 1,
                                                                        [](std::size_t x, std::size_t y) {
                                                                            return x * y;
                                                                        }));
                break;
            case Type::Reference:
                cached_llvm_type = llvm::PointerType::get(subtype->get_llvm_type(ctx), ctx->machine.data_pointer_addr);
                break;
            case Type::Pointer:
                cached_llvm_type = llvm::PointerType::get(subtype->get_llvm_type(ctx), ctx->machine.data_pointer_addr);
                break;
            case Type::Object: {
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                cached_llvm_type = struct_type;
                auto vec = std::vector<llvm::Type *>{};
                for (const auto &ty: child_types) {
                    vec.push_back(ty->get_llvm_type(ctx));
                }
                auto arr = llvm::ArrayRef<llvm::Type *>{vec};
                struct_type->setBody(arr);
                break;
            }
            case Type::WeakReference: {
                auto ref = weak_reference.lock();
                cached_llvm_type = llvm::PointerType::get(ref->get_llvm_type(ctx), ctx->machine.data_pointer_addr);
                break;
            }
            case Type::WeakPointer: {
                auto ref = weak_reference.lock();
                cached_llvm_type = llvm::PointerType::get(ref->get_llvm_type(ctx), ctx->machine.data_pointer_addr);
                break;
            }
            case Type::WeakSlice: {
                auto ref = weak_reference.lock();
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                cached_llvm_type = struct_type;
                struct_type->setBody(llvm::IntegerType::get(ctx->llvm_context, ctx->machine.data_pointer_size * 8),
                                     llvm::PointerType::get(ref->get_llvm_type(ctx), ctx->machine.data_pointer_addr));
                break;
            }
        }
        return cached_llvm_type;
    }

    size_t BacteriaType::get_llvm_size(cheese::project::GlobalContext *ctx) {
        return ctx->machine.layout.getTypeAllocSize(get_llvm_type(ctx));
    }
}