//
// Created by Lexi Allen on 4/23/2023.
//
#include "bacteria/BacteriaType.h"
#include "project/GlobalContext.h"
#include <sstream>

namespace cheese::bacteria {

    std::string bacteria::BacteriaType::to_string(bool emitFirstLayer) {
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
                return "<>" + std::string(constant_ref ? "~" : "") + subtype->to_string();
            case Type::Array: {
                std::stringstream ss{};
                ss << "[";
                for (int i = 0; i < array_dimensions.size(); i++) {
                    ss << array_dimensions[i];
                    if (i < array_dimensions.size() - 1) {
                        ss << ",";
                    }
                }
                ss << "]" << std::string(constant_ref ? "~" : "") << subtype->to_string();
                return ss.str();
            }
            case Type::Reference:
                return "*" + std::string(constant_ref ? "~" : "") + subtype->to_string();
            case Type::Pointer:
                return "[*]" + std::string(constant_ref ? "~" : "") + subtype->to_string();
            case Type::Object: {
                if (emitFirstLayer || struct_name.empty()) {
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
                } else {
                    return struct_name;
                }
            }
            case Type::UnsignedSize:
                return "usize";
            case Type::SignedSize:
                return "isize";
            case Type::FunctionPointer: {
                std::stringstream ss{};
                ss << "fn(";
                for (int i = 0; i < child_types.size(); i++) {
                    ss << child_types[i]->to_string();
                    if (i < child_types.size() - 1) {
                        ss << ",";
                    }
                }
                ss << ")=>" << subtype->to_string();
                return ss.str();
            }
            default:
                return "unknown";
        }
    }

    BacteriaType::BacteriaType(Type type, uint16_t integerSize,
                               BacteriaType *subtype,
                               const std::vector<std::size_t> &arrayDimensions,
                               const std::vector<BacteriaType *> &childTypes,
                               const std::string &structName,
                               const bool constant_ref) : type(type), integer_size(
            integerSize), subtype(subtype), array_dimensions(arrayDimensions), child_types(childTypes),
                                                          struct_name(structName),
                                                          constant_ref(constant_ref) {}

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
                struct_type->setName("c32");
                struct_type->setBody(llvm::ArrayRef<llvm::Type *>{llvm::Type::getFloatTy(ctx->llvm_context),
                                                                  llvm::Type::getFloatTy(ctx->llvm_context)}, true);
                break;
            }
            case Type::Complex64: {
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                cached_llvm_type = struct_type;
                struct_type->setName("c64");
                struct_type->setBody(llvm::ArrayRef<llvm::Type *>{llvm::Type::getDoubleTy(ctx->llvm_context),
                                                                  llvm::Type::getDoubleTy(ctx->llvm_context)}, true);
                break;
            }
            case Type::Slice: {
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                cached_llvm_type = struct_type;
                struct_type->setBody(llvm::IntegerType::get(ctx->llvm_context, ctx->machine.data_pointer_size * 8),
                                     llvm::PointerType::get(ctx->llvm_context, ctx->machine.data_pointer_addr));
                break;
            }
            case Type::Array:
                for (std::ptrdiff_t i = array_dimensions.size() - 1; i >= 0; i--) {
                    if (cached_llvm_type != nullptr) {
                        cached_llvm_type = llvm::ArrayType::get(cached_llvm_type, array_dimensions[i]);
                    } else {
                        cached_llvm_type = llvm::ArrayType::get(subtype->get_llvm_type(ctx), array_dimensions[i]);
                    }
                }

                break;
            case Type::Reference:
            case Type::Pointer:
            case Type::FunctionPointer:
                cached_llvm_type = llvm::PointerType::get(ctx->llvm_context, ctx->machine.data_pointer_addr);
                break;
            case Type::Object: {
                auto struct_type = llvm::StructType::create(ctx->llvm_context);
                if (!struct_name.empty()) struct_type->setName(struct_name);
                cached_llvm_type = struct_type;
                auto vec = std::vector<llvm::Type *>{};
                for (const auto &ty: child_types) {
                    vec.push_back(ty->get_llvm_type(ctx));
                }
                auto arr = llvm::ArrayRef<llvm::Type *>{vec};
                struct_type->setBody(arr);
                break;
            }
        }
        return cached_llvm_type;
    }

    size_t BacteriaType::get_llvm_size(cheese::project::GlobalContext *ctx) {
        return ctx->machine.layout.getTypeAllocSize(get_llvm_type(ctx));
    }

    bool BacteriaType::matches(BacteriaType::Type otherType, uint16_t integerSize, BacteriaType *otherSubtype,
                               const std::vector<std::size_t> &arrayDimensions,
                               const std::vector<BacteriaType *> &childTypes, const std::string &structName,
                               const bool constRef) {
        if (otherType != this->type) return false;
        switch (type) {
            case Type::Opaque:
            case Type::Noreturn:
            case Type::Void:
            case Type::UnsignedSize:
            case Type::SignedSize:
            case Type::Float32:
            case Type::Float64:
            case Type::Complex32:
            case Type::Complex64:
                return true;
            case Type::SignedInteger:
            case Type::UnsignedInteger:
                return integer_size == integerSize;
            case Type::Slice:
            case Type::Reference:
            case Type::Pointer:
                return subtype == otherSubtype;
            case Type::Array:
                return arrayDimensions == array_dimensions && subtype == otherSubtype;
            case Type::Object:
                if (structName != struct_name) return false;
                if (struct_name.empty()) {
                    return true;
                } else {
                    return true; // As these kinda have to be the same, struct names are globally unique
                }
            case Type::FunctionPointer: {
                if (childTypes.size() != child_types.size()) return false;
                for (auto i = 0; i < childTypes.size(); i++) {
                    if (childTypes[i] != child_types[i]) return false;
                }
                return subtype == otherSubtype && constant_ref == constRef;
            }
        }
    }

    bool BacteriaType::should_implicit_reference() {
        switch (type) {
            case Type::Opaque:
            case Type::Void:
            case Type::Noreturn:
            case Type::UnsignedInteger:
            case Type::UnsignedSize:
            case Type::SignedInteger:
            case Type::SignedSize:
            case Type::Float32:
            case Type::Float64:
            case Type::Complex32:
            case Type::Complex64:
            case Type::Slice:
            case Type::Reference:
            case Type::Pointer:
            case Type::FunctionPointer:
                return false;
            case Type::Array:
            case Type::Object:
                return true;
        }
    }

    bool BacteriaType::is_same_as(BacteriaType *other) {
        return matches(other->type, other->integer_size, other->subtype, other->array_dimensions, other->child_types,
                       other->struct_name, other->constant_ref);
    }

    bacteria::BacteriaType *BacteriaType::index_type(nodes::BacteriaProgram *program, std::size_t numIndices) {
        if (numIndices == 0) return this;
        switch (type) {
            case Type::Slice:
            case Type::Pointer:
                return subtype->index_type(program, numIndices - 1);
            case Type::Array:
                if (numIndices >= array_dimensions.size()) {
                    return subtype->index_type(program, numIndices - array_dimensions.size());
                } else {
                    std::vector<std::size_t> newDimensions;
                    for (int i = array_dimensions.size() - 1; i >= numIndices; i--) {
                        newDimensions.insert(newDimensions.begin(), array_dimensions.size());
                    }
                    return program->get_type(Type::Reference, 0,
                                             program->get_type(Type::Array, 0, subtype, newDimensions, {}, {},
                                                               constant_ref), {}, {}, {}, constant_ref);
                }
            default:
                return this;
        }
    }
}