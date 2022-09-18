//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_LEXER_H
#define CHEESE_LEXER_H

#include "../Coordinate.h"
namespace cheese::lexer {
    enum class TokenType {

        //Structural tokens
        SingleArrow, // ->
        DoubleArrow, // -->
        ThickArrow, // =>
        DoubleThickArrow, // ==>
        ReversedDoubleThickArrow, // <== (used for yielding, <== none means yield nothing (to yield the none type its <== (none) in parentheses))
        ReversedArrow, //<-
        LeftBrace, // {
        RightBrace, // }
        LeftBracket, // [
        RightBracket, // ]
        LeftParen, // (
        RightParen, // )
        Colon, // :
        // DoubleColon, // ::
        Comma, // ,
        Semicolon, // ; (used to denote the end of function calls a! b;
        // Different syntax for function calls to make parsing easier
        // a.b.c.d.e.f.g.h.i.j.k! e;
        Dot, // .
        Dot2, // .. Used for ranges in subslicing slices
        Dot3, // ... Used for ranges in ranged for loops
        //Special tokens
        Hash, // # Used for finding out the length of an array or slice
        Question, // ? Used for denoting an uknown sized array of items
        ConstPointer, // *~
        ConstArray, // ]~
        ConstSlice, // >~
        Cast, // @
        //Operator tokens
        Star, // * (Used for dereferencing, pointers, and multiplication)
        Percent, // % Modulo
        Slash, // / Divide
        Plus, // + Add
        Dash, // - Subtract
        Ampersand, // & Reference
        LeftShift, // <<
        RightShift, // >>
        GreaterThan, // >
        LessThan, // <
        EqualTo, // ==
        NotEqualTo, // !=
        GreaterThanEqual, // >=
        LessThanEqual, // <=
        BooleanAnd, // &&
        BooleanOr, // ||
        BooleanNot, // !
        BitwiseOr, // |
        BitwiseXor, // ^
        BitwiseNot, // ~
        Assign, // =
        MulAssign, // *=
        ModAssign, // +=
        DivAssign, // /=
        AddAssign, // +=
        SubAssign, // -=
        BooleanAndAssign, // &&=
        BooleanOrAssign, // ||=
        LeftShiftAssign, // <<=
        RightShiftAssign, // >>=
        BitwiseAndAssign, // &=
        BitwiseOrAssign, // |=
        BitwiseXorAssign, // ^=
        Unwrap, // # (This is a postfix unwrap operator for types that are wrapped like a pointer or a unit in the future)
        Dereference, // $
        Call, // !(
        Object, // !{
        Exponentiate, // ^^
        ExponentiateAssign, //^^=

        //Textual tokens
        Public, // public
        Private, // private
        Const, // const
        NoReturn, // noreturn
        Member, // member
        Enum, // enum
        Union, // union
        Struct, // struct
        Extern, // extern
        Let, // let
        Def, // def (used for defining an uninitialized variable, the same as 'let none --> ...' for non extern variables)
        Fn, // fn
        Unpack, // unpack ... --> ...
        Float32, //f32
        Float64, //f64
        Complex32, //c32
        Complex64, //c64
        Switch, //switch
        If, //if
        Elif, //elif
        Else, //else
        While, //while
        For, //for
        Import, //import
        Type, //type
        None, //none
        Void, //void
        Discard, //discard
        //Namespace, //namespace
        Prototype, //prototype
        Inline, //inline
        Entry, //entry
        Comptime, //comptime (used for compile time code execution or variables)
        Break, //break Shorthand for <== none
        Continue, //continue goes to the next iteration
        Any, //any (used for any type at compile time, is reduced later)
        Asm, //asm (used for inline assembly)
        ComptimeString, //cstr (A compiletime string type that is the type of all string literals)
        Expression, //Expression (A type that stores expressions (ASTNodes) at compiletime)
        //also contains a dictionary at compile time containing compile time known values to interpolate an expression at compile time (which actually creates a new expression value so the old one can still be used)
        //there will be builtin functions for creating larger AST Node types, to the point where entire functions and libraries could be generated at compile time, including upto creating a compile time translator/compiler for another language
        //an expr literal is denoted by %expr
        Expressions, //exprs (A compiletime list of expressions, where each one is individually interpolated and the list as whole is also interpolated)
        //Can be used to inject a list of expressions at compile time
        //Exprs are denoted by %(
        //  expr
        //  expr
        //  expr
        //  ...
        //)
        Statement, //stmnt (A compiletime body statement, used to inject statements at compile time
        //denoted by %[statement]
        Statements, //stmnts (A compiletime list of body statements)
        //(There will be functions to add or remove from this, the adding and removing will add or remove interpolated body statements, as each seperate statement will be individually interpolated)
        //Denoted by %{
        //
        //}
        //An expr can be cast into a stmnt but not the other way around
        //And also an exprs value can be cast into a stmnts type but not the other way around
        ComptimeFloat, //comptime_float
        ComptimeInt, //comptime_int
        ComptimeComplex, //comptime_complex
        UnsignedSize, //usize, gets translated to the correct type when being lowered, for now assume 8 bytes
        SignedSize, //isize
        //dynamic tokens
        SignedIntType, //i[0-9]+
        UnsignedIntType, //u[0-9]+
        ImaginaryLiteral, //[float]I
        FloatingLiteral, //[0-9]?.[0-9]+
        DecimalLiteral, //[0-9]+
        HexLiteral, //0x[0-9a-fA-F]+
        OctalLiteral, //0o[0-7]+
        BinaryLiteral, //0b[01_]+
        Identifier, //[a-zA-Z_][a-zA-Z0-9_]*
        BuiltinReference, //$[a-zA-Z_][a-zA-Z0-9_]*
        StringLiteral,
        //MultilineStringLiteral, //```...``` gets dedented and trimmed automatically by the parser
        CharacterLiteral,
        SingleLineComment,
        BlockComment,
        Underscore,
        //Special Tokens
        Error, //An error token in the lexer
        EoF, //End of File
    };

    struct Token {
        Coordinate location;
        TokenType ty;
        std::string value;
    };
    std::vector<Token> lex(std::string buffer, std::string filename="unknown");
} //cheese::lexer
#endif //CHEESE_LEXER_H
