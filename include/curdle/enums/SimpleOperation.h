//
// Created by Lexi Allen on 9/30/2023.
//

#ifndef CHEESE_SIMPLEOPERATION_H
#define CHEESE_SIMPLEOPERATION_H

#include <string>

namespace cheese::curdle::enums {
    enum class SimpleOperation {
        UnaryPlus, // +a ==> +(a(...))
        UnaryMinus, // -a ==> -(a(...))
        Not, // not a ==> not (a(...))
        Multiplication, // a * b ==> a(...) * b(...)
        Division,
        Remainder,
        Addition,
        Subtraction,
        LeftShift,
        RightShift,
        LesserThan,
        GreaterThan,
        LesserThanOrEqualTo,
        GreaterThanOrEqualTo,
        EqualTo,
        NotEqualTo,
        And,
        Or,
        Xor,
        Combine, // a & b ==> a(...) & b(...)
        Compose, // a | b ==> b(a(...))
    };

    bool is_binary_op(SimpleOperation op);

    std::string get_op_string(SimpleOperation op);
}
#endif //CHEESE_SIMPLEOPERATION_H
