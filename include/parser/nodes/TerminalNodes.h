//
// Created by Lexi Allen on 11/8/2022.
//

#ifndef CHEESE_TERMINALNODES_H
#define CHEESE_TERMINALNODES_H

#include "../Node.h"
#include "NotImplementedException.h"
namespace cheese::parser::nodes {
    TERMINAL_NODE(Bool, "bool")
    TERMINAL_NODE(SignedSize, "isize_type")
    TERMINAL_NODE(UnsignedSize, "usize_type")
    TERMINAL_NODE(Float32, "f32_type")
    TERMINAL_NODE(Float64, "f64_type")
    TERMINAL_NODE(Complex32, "c32_type")
    TERMINAL_NODE(Complex64, "c64_type")
    TERMINAL_NODE(Opaque, "opaque_type")
    TERMINAL_NODE(Void, "void_type")
    TERMINAL_NODE(ComptimeFloat, "comptime_float_type")
    TERMINAL_NODE(ComptimeComplex, "comptime_complex_type")
    TERMINAL_NODE(ComptimeString, "comptime_string_type")
    TERMINAL_NODE(ComptimeInt, "comptime_int_type")
    TERMINAL_NODE(Type, "type_type")
    TERMINAL_NODE(NoReturn, "noreturn_type")
    TERMINAL_NODE(True, "true")
    TERMINAL_NODE(False, "false")
    TERMINAL_NODE(None, "none")
    TERMINAL_NODE(Underscore, "underscore")
    TERMINAL_NODE(Continue, "continue")
    TERMINAL_NODE(EmptyBreak, "empty_break")
    TERMINAL_NODE(EmptyReturn, "empty_return")
}

#endif //CHEESE_TERMINALNODES_H
