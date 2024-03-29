//
// Created by Lexi Allen on 11/8/2022.
//

#ifndef CHEESE_SINGLE_MEMBER_NODES_H
#define CHEESE_SINGLE_MEMBER_NODES_H

#include "../Node.h"
#include "NotImplementedException.h"
#include "math/BigInteger.h"


namespace cheese::parser::nodes {
    //Single member nodes without a specific value

    //i[size]
    SINGLE_MEMBER_NODE(SignedIntType, "int_type", std::uint64_t, size)
    //u[size]
    SINGLE_MEMBER_NODE(UnsignedIntType, "uint_type", std::uint64_t, size)
    //"..."
    SINGLE_MEMBER_NODE(StringLiteral, "string", std::string, str)
    //[integer_literal]
    SINGLE_MEMBER_NODE(IntegerLiteral, "int", math::BigInteger, value)
    //[float_literal]
    SINGLE_MEMBER_NODE(FloatLiteral, "float", double, value)
    //[float_literal]I
    SINGLE_MEMBER_NODE(ImaginaryLiteral, "imaginary", double, value)
    //[identifier]
    SINGLE_MEMBER_NODE(ValueReference, "ref", std::string, name)
    //{...}
    SINGLE_MEMBER_NODE(UnnamedBlock, "unnamed_block", NodeList, children)
    //.(...)
    SINGLE_MEMBER_NODE(TupleLiteral, "tuple_literal", NodeList, children)
    //.[...]
    SINGLE_MEMBER_NODE(ArrayLiteral, "array_literal", NodeList, children)
    //.[identifier]
    SINGLE_MEMBER_NODE(EnumLiteral, "enum_literal", std::string, name)
    //$[identifier]
    SINGLE_MEMBER_NODE(BuiltinReference, "builtin", std::string, builtin)

    SINGLE_MEMBER_NODE(CopyCapture, "copy_capture", std::string, name)

    SINGLE_MEMBER_NODE(RefCapture, "ref_capture", std::string, name)

    SINGLE_MEMBER_NODE(ConstRefCapture, "const_ref_capture", std::string, name)

    SINGLE_MEMBER_NODE(ObjectLiteral, "object_literal", NodeList, children)


    SINGLE_MEMBER_NODE(Block, "block", NodeList, children)

    SINGLE_MEMBER_NODE(MatchValue, "match_value", NodePtr, value)

    SINGLE_MEMBER_NODE(MatchConstraint, "match_constraint", NodePtr, constraint)

    SINGLE_MEMBER_NODE(DestructuringMatchStructure, "destructuring_match_structure", NodeDict, children)

    SINGLE_MEMBER_NODE(DestructuringMatchTuple, "destructuring_match_tuple", NodeList, children)

    SINGLE_MEMBER_NODE(DestructuringMatchArray, "destructuring_match_array", NodeList, children)


    //Single child nodes
    //<== [value]
    SINGLE_CHILD_NODE(Break, "break")
    //==> [value]
    SINGLE_CHILD_NODE(Return, "return")
    //not [value]
    SINGLE_CHILD_NODE(Not, "not")
    //-[value]
    SINGLE_CHILD_NODE(UnaryMinus, "unary_minus")
    //+[value]
    SINGLE_CHILD_NODE(UnaryPlus, "unary_plus")
    //[value]$
    SINGLE_CHILD_NODE(Dereference, "dereference")
    //&[value]
    SINGLE_CHILD_NODE(AddressOf, "address_of")
    //concept [statement]
    SINGLE_CHILD_NODE(Concept, "concept")
    //constrain [functional]
    SINGLE_CHILD_NODE(Constrain, "constrain")
    //loop [expression]
    SINGLE_CHILD_NODE(Loop, "loop")
    //? expression
    SINGLE_CHILD_NODE(FilterTransformation, "filter_transformation")
    //: expression
    SINGLE_CHILD_NODE(MapTransformation, "map_transformation")


    SINGLE_CHILD_NODE(Comptime, "comptime")


    //Technically single child, with a possible const qualifier
    //[](~)[subtype]
    POSSIBLY_CONST_TYPE_NODE(ImplicitArray, "implicit_array")
    //<>(~)[subtype]
    POSSIBLY_CONST_TYPE_NODE(Slice, "slice")
    //*(~)[subtype]
    POSSIBLY_CONST_TYPE_NODE(Reference, "reference")
}


#endif //CHEESE_SINGLE_MEMBER_NODES_H
