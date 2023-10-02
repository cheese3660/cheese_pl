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

namespace cheese::project {
    struct GlobalContext;
}

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
            FunctionPointer, // subtype == return type, child_types == argument types
        } type = Type::Void;
        std::uint16_t integer_size = 0;
        BacteriaType *subtype = {};
        std::vector<std::uint64_t> array_dimensions = {};
        std::vector<BacteriaType *> child_types = {}; //Used for structures, all pointers to objects are replaced with opaque pointers
        std::string struct_name{};
        // If this is empty, there is no name, otherwise there is
        // Named structure types will be emitted as text to the top of bacteria file

        explicit BacteriaType(Type type = Type::Void, uint16_t integerSize = 0,
                              BacteriaType *subtype = {},
                              const std::vector<std::size_t> &arrayDimensions = {},
                              const std::vector<BacteriaType *> &childTypes = {},
                              const std::string &structName = {},
                              const bool constant_ref = {});

        BacteriaType &operator=(const BacteriaType &other) = default;

        BacteriaType(const BacteriaType &other) = default;

        std::string to_string(bool emitFirstLayer = false);

        llvm::Type *get_llvm_type(cheese::project::GlobalContext *ctx);

        size_t get_llvm_size(cheese::project::GlobalContext *ctx);

        bool constant_ref; // If the function pointer is extern or not

        bool
        matches(Type otherType, uint16_t integerSize, BacteriaType *subtype,
                const std::vector<std::size_t> &arrayDimensions,
                const std::vector<BacteriaType *> &childTypes, const std::string &structName, const bool externFn);

        bool is_same_as(BacteriaType *other);

        bool should_implicit_reference();

    private:
        llvm::Type *cached_llvm_type{nullptr};
    };

    typedef BacteriaType *TypePtr;
    typedef std::vector<TypePtr> TypeList;
    typedef std::map<std::string, TypePtr> TypeDict;
}
#endif //CHEESE_BACTERIATYPE_H
