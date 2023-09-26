//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_BACTERIATYPE_H
#define CHEESE_BACTERIATYPE_H

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <llvm/IR/Type.h>
#include "project/Machine.h"

namespace cheese::bacteria {
    struct BacteriaType {

        enum class Type {
            Opaque,
            Void,
            Noreturn,
            UnsignedInteger,
            UnsignedSize,
            SignedInteger,
            SignedSize,
            Float32,
            Float64,
            Complex32,
            Complex64,
            Slice,
            Array,
            Reference,
            Pointer,
            Object,
            WeakReference,
            WeakPointer,
            WeakSlice,
        } type = Type::Void;
        std::uint16_t integer_size = 0;
        std::shared_ptr<BacteriaType> subtype = {};
        std::vector<std::size_t> array_dimensions = {};
        std::vector<std::shared_ptr<BacteriaType>> child_types = {}; //Used for structures, all pointers to objects are replaced with opaque pointers
        std::weak_ptr<BacteriaType> weak_reference = {};

        BacteriaType(Type type = Type::Void, uint16_t integerSize = 0,
                     const std::shared_ptr<BacteriaType> &subtype = {},
                     const std::vector<std::size_t> &arrayDimensions = {},
                     const std::vector<std::shared_ptr<BacteriaType>> &childTypes = {},
                     const std::weak_ptr<BacteriaType> &weak_reference = {});

        BacteriaType &operator=(const BacteriaType &other) = default;

        BacteriaType(const BacteriaType &other) = default;

        std::string to_string();

        llvm::Type *get_llvm_type(const curdle::Machine &machine);
    };

    typedef std::shared_ptr<BacteriaType> TypePtr;
    typedef std::vector<TypePtr> TypeList;
    typedef std::map<std::string, TypePtr> TypeDict;
}
#endif //CHEESE_BACTERIATYPE_H
