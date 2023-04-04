//
// Created by Lexi Allen on 11/8/2022.
//

#ifndef CHEESE_TERMINAL_NODES_H
#define CHEESE_TERMINAL_NODES_H

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

    TERMINAL_NODE(AnyType, "any")

    TERMINAL_NODE(ComptimeFloat, "comptime_float")

    TERMINAL_NODE(ComptimeComplex, "comptime_complex")

    TERMINAL_NODE(ComptimeString, "comptime_string")

    TERMINAL_NODE(ComptimeInt, "comptime_int")

    TERMINAL_NODE(Type, "type_type")

    TERMINAL_NODE(NoReturn, "noreturn_type")

    TERMINAL_NODE(True, "true")

    TERMINAL_NODE(False, "false")

    TERMINAL_NODE(None, "none")

    TERMINAL_NODE(Underscore, "underscore")

    TERMINAL_NODE(Continue, "continue")

    TERMINAL_NODE(EmptyBreak, "empty_break")

    TERMINAL_NODE(EmptyReturn, "empty_return")

    TERMINAL_NODE(Self, "self")

    TERMINAL_NODE(ConstSelf, "const_self")

    TERMINAL_NODE(SelfType, "self_type")

    TERMINAL_NODE(ConstRefImplicitCapture, "const_ref_capture_implicit")

    TERMINAL_NODE(RefImplicitCapture, "ref_capture_implicit")

    TERMINAL_NODE(CopyImplicitCapture, "copy_capture_implicit")

    TERMINAL_NODE(MatchAll, "match_all")

    TERMINAL_NODE(NonExhaustive, "non_exhaustive")

    TERMINAL_NODE(UnknownSize, "unknown_size")

    TERMINAL_NODE(InferredSize, "inferred_size")
}

#endif //CHEESE_TERMINAL_NODES_H
