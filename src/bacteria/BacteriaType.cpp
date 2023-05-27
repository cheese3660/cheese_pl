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
}