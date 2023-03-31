//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_BACTERIATYPE_H
#define CHEESE_BACTERIATYPE_H

#include <memory>

namespace cheese::bacteria {
    struct BacteriaType {
        enum class Type {
            Opaque,
            Void,
            Noreturn,
            UnsignedInteger,
            SignedInteger,
            Float32,
            Float64,
            Complex32,
            Complex64,
            Slice,
            Array,
            Reference,
            Pointer,
            Object
        } type;
        std::uint16_t integer_size;
        std::shared_ptr<BacteriaType> subtype;
        std::vector<std::size_t> array_dimensions;
        std::vector<std::shared_ptr<BacteriaType>> child_types; //Used for structures, all pointers to objects are replaced with opaque pointers
    };
    typedef std::shared_ptr<BacteriaType> TypePtr;
    typedef std::vector<TypePtr> TypeList;
    typedef std::map<std::string, TypePtr> TypeDict;
}
#endif //CHEESE_BACTERIATYPE_H
