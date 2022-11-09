//
// Created by Lexi Allen on 11/8/2022.
//

#ifndef CHEESE_SINGLEMEMBERNODES_H
#define CHEESE_SINGLEMEMBERNODES_H

#include "../Node.h"
#include "NotImplementedException.h"
#include "math/BigInteger.h"


namespace cheese::parser::nodes {
    //Single member nodes without a specific value

    SINGLE_MEMBER_NODE(SignedIntType,"int_type",std::uint64_t,"size")
    SINGLE_MEMBER_NODE(UnsignedIntType,"uint_type",std::uint64_t,"size")
    SINGLE_MEMBER_NODE(StringLiteral,"string",std::string,"str")
    SINGLE_MEMBER_NODE(IntegerLiteral,"int",math::BigInteger,"value")
}


#endif //CHEESE_SINGLEMEMBERNODES_H
