//
// Created by Lexi Allen on 9/18/2022.
//

#include "lexer/lexer.h"
#include "NotImplementedException.h"
#include "error.h"
#include <optional>
#include <functional>
#include <algorithm>
#include <utility>
#include <iostream>
#include <sstream>
namespace cheese::lexer {
#define RESERVED(keyword, token) std::pair<std::string_view, TokenType>{# keyword, token},
    const auto reserved_keywords = std::vector<std::pair<std::string_view, TokenType>>{
        RESERVED(public,TokenType::Public)
        RESERVED(private,TokenType::Private)
        RESERVED(mut,TokenType::Mutable)
        RESERVED(noreturn, TokenType::NoReturn)
        RESERVED(member, TokenType::Member)
        RESERVED(enum, TokenType::Enum)
        RESERVED(union, TokenType::Union)
        RESERVED(struct, TokenType::Struct)
        RESERVED(extern, TokenType::Extern)
        RESERVED(let, TokenType::Let)
        RESERVED(def, TokenType::Def)
        RESERVED(fn, TokenType::Fn)
        RESERVED(unpack, TokenType::Unpack)
        RESERVED(f32, TokenType::Float32)
        RESERVED(f64, TokenType::Float64)
        RESERVED(c32, TokenType::Complex32)
        RESERVED(c64, TokenType::Complex64)
        RESERVED(match, TokenType::Match)
        RESERVED(if, TokenType::If)
        RESERVED(elif, TokenType::Elif)
        RESERVED(else, TokenType::Else)
        RESERVED(while, TokenType::While)
        RESERVED(for, TokenType::For)
        RESERVED(type, TokenType::Type)
        RESERVED(none, TokenType::None)
        RESERVED(void, TokenType::Void)
        RESERVED(discard, TokenType::Discard)
        RESERVED(import, TokenType::Import)
        RESERVED(prototype, TokenType::Prototype)
        RESERVED(inline, TokenType::Inline)
        RESERVED(entry, TokenType::Entry)
        RESERVED(any, TokenType::Any)
        RESERVED(comptime, TokenType::Comptime)
        RESERVED(comptime_string, TokenType::ComptimeString)
        RESERVED(asm, TokenType::Asm)
        RESERVED(comptime_float, TokenType::ComptimeFloat)
        RESERVED(comptime_complex, TokenType::ComptimeComplex)
        RESERVED(comptime_int, TokenType::ComptimeInt)
        RESERVED(usize, TokenType::UnsignedSize)
        RESERVED(isize, TokenType::SignedSize)
        RESERVED(continue,TokenType::Continue)
        RESERVED(break, TokenType::Break)
        RESERVED(_,TokenType::Underscore)
        RESERVED(then,TokenType::Then)
        RESERVED(do,TokenType::Do)
        RESERVED(and,TokenType::And)
        RESERVED(and=,TokenType::AndAssign)
        RESERVED(or,TokenType::Or)
        RESERVED(or=,TokenType::OrAssign)
        RESERVED(xor,TokenType::Xor)
        RESERVED(xor=,TokenType::XorAssign)
        RESERVED(not,TokenType::Not)
        RESERVED(bool,TokenType::Bool)
        RESERVED(true,TokenType::True)
        RESERVED(false,TokenType::False)
        RESERVED(generator,TokenType::Generator)
        RESERVED(is,TokenType::Is)
        RESERVED(opaque,TokenType::Opaque)
        RESERVED(export,TokenType::Export)
        RESERVED(impl, TokenType::Impl)
        RESERVED(interface, TokenType::Interface)
        RESERVED(dynamic, TokenType::Dynamic)
        RESERVED(with, TokenType::With)
        RESERVED(constrain,TokenType::Constrain)
    };
#undef  RESERVED
    const auto builtin_macros = std::vector<std::string_view>{
        "try",
        "Err",
        "Ok",
        "Result",
    };
    std::optional<TokenType> getKW(std::string_view kw) {
        for (const auto & reserved_keyword : reserved_keywords) {
            if (reserved_keyword.first == kw) {
                return reserved_keyword.second;
            }
        }
        return std::optional<TokenType>{};
    }
    bool isMacro(std::string_view macro) {
//        for (auto builtin_macro : builtin_macros) {
//            if (builtin_macro == macro) return true;
//        }
//        return false;
        return std::any_of(builtin_macros.begin(),builtin_macros.end(),[macro](std::string_view str){return str == macro;});
    }
    struct _lexerState {
        std::string filename;
        std::string_view buffer;
        std::size_t buffer_position=0;
        std::uint32_t column=1;
        std::uint32_t line=1;
        [[nodiscard]] char peek() const {
            if (buffer_position < buffer.size()) {
                return buffer[buffer_position];
            } else {
                return 0; //0 signifies END
            }
        }
        void advance() {
            if (peek() != '\n') {
                column++;
            } else {
                column = 1;
                line++;
            }
            buffer_position++;
        }

        [[nodiscard]] bool eof() const {
            return buffer_position >= buffer.size();
        }

        Coordinate location() {
            return Coordinate{line,column, getFileIndex(filename)};
        }
    };
    static inline bool validB(char c) {
        return (c == '0' || c == '1') || c == '_';
    }
    static inline bool validO(char c) {
        return (c >= '0' && c <= '7') || c == '_';
    }
    static inline bool validX(char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == '_';
    }
    static inline bool validDBeg(char c) {
        return (c >= '0' && c <= '9');
    }
    static inline bool validDMid(char c) {
        return (c >= '0' && c <= '9') || c == '_';
    }
    static inline bool validIBeg(char c) {
        return std::isalpha(c) || c == '_';
    }
    static inline bool validIMid(char c) {
        return std::isalnum(c) || c == '_';
    }
    static bool isNumber(std::string_view sv) {
        if (sv.empty()) return false;
        return std::all_of(sv.begin(),sv.end(),[](char c) { return std::isdigit(c);});
    }

    static bool validIntegerType(std::string_view ty) {
        if (ty[0] == 'u' || ty[0] == 'i') {
            return isNumber(ty.substr(1));
        }
        return false;
    }
#define PEEK __state.peek()
#define ADVANCE __state.advance()
#define LINE __state.line()
#define COLUMN __state.column()
#define POSITION __state.buffer_position
#define LOCATION __state.location()
#define VIEW buffer.substr(view_start,view_size)
#define ADD(type) tokens.push_back(Token{start_location, type, VIEW})
#define SINGLE(type) ADVANCE; view_size++; ADD(type)
#define END __state.eof()
    std::vector<Token> lex(std::string_view buffer, std::string filename, bool errorInvalid, bool outputComments, bool warnComments) {
        _lexerState __state = _lexerState{std::move(filename),buffer,0};
        std::vector<Token> tokens{};
        std::function<Token(Coordinate,std::size_t)> skipComment = [&](Coordinate start_location, std::size_t view_start) -> Token {
            std::size_t view_size = 2; // '//'
            while (!END) {
                auto current = PEEK;
                ADVANCE;
                if (current != '\n') {
                    view_size += 1;
                } else {
                    break;
                }
            }
            std::string_view comment = VIEW;
            if (warnComments) {
                if (comment.contains("TODO")) {
                    error::raise_warning("LEXER", "'TODO' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::TodoFoundInComment);
                }
                if (comment.contains("FIXME")) {
                    error::raise_warning("LEXER", "'FIXME' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::FixmeFoundInComment);
                }
                if (comment.contains("BUG")) {
                    error::raise_warning("LEXER", "'BUG' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::BugFoundInComment);
                }
                if (comment.contains("XXX")) {
                    error::raise_warning("LEXER", "'XXX' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::XXXFoundInComment);
                }
                if (comment.contains("HACK")) {
                    error::raise_warning("LEXER", "'HACK' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::HackFoundInComment);
                }
            }
            return Token{
                start_location,
                TokenType::SingleLineComment,
                comment,
            };
        };
        std::function<Token(Coordinate,std::size_t)> skipMultiLineComment = [&](Coordinate start_location, std::size_t view_start) -> Token {
            std::size_t view_size = 2; // '/*'
            while (true) {
                if (END) {
                    if (errorInvalid) {
                        error::raise_exiting_error("LEXER","unterminated block comment", start_location, error::ErrorCode::UnterminatedMultilineComment);
                    } else {
                        return Token{
                            start_location,
                            TokenType::Error,
                            buffer.substr(view_start,view_size),
                        };
                    }
                }
                auto currentLocation = LOCATION;
                auto current = PEEK;
                view_size += 1;
                switch (current) {
                    case '*': {
                        ADVANCE;
                        auto lookahead = PEEK;
                        if (lookahead == '/') {
                            view_size += 1;
                            ADVANCE;
                            goto loopEnd;
                        }
                        break;
                    }
                    case '/': {
                        ADVANCE;
                        auto lookahead = PEEK;
                        if  (lookahead == '*') {
                            view_size += 1;
                            ADVANCE;
                            auto res = skipMultiLineComment(currentLocation, POSITION);
                            view_size += res.value.size();
                        }
                        break;
                    }
                    default:
                        ADVANCE;
                        break;
                }
            }
            loopEnd:
            std::string_view comment = VIEW;
            if (warnComments) {
                if (comment.contains("TODO")) {
                    error::raise_warning("LEXER", "'TODO' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::TodoFoundInComment);
                }
                if (comment.contains("FIXME")) {
                    error::raise_warning("LEXER", "'FIXME' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::FixmeFoundInComment);
                }
                if (comment.contains("BUG")) {
                    error::raise_warning("LEXER", "'BUG' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::BugFoundInComment);
                }
                if (comment.contains("XXX")) {
                    error::raise_warning("LEXER", "'XXX' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::XXXFoundInComment);
                }
                if (comment.contains("HACK")) {
                    error::raise_warning("LEXER", "'HACK' found inside a comment, maybe you should look into this?", start_location, error::ErrorCode::HackFoundInComment);
                }
            }
            return Token{
                    start_location,
                    TokenType::BlockComment,
                    comment,
            };
        };
        std::function<Token()> str = [&]() -> Token {
            auto view_start = POSITION;
            auto location = LOCATION;
            std::size_t view_size = 1;
            auto in_escape = false;
            bool running = true;
            ADVANCE;
            while (running) {
                auto current = PEEK;
                if (END || current == '\n') {
                    if (errorInvalid) {
                        error::raise_exiting_error("LEXER", "unterminated string literal", location,error::ErrorCode::UnterminatedStringLiteral);
                    } else {
                        return Token{LOCATION, TokenType::Error, VIEW};
                    }
                }
                ADVANCE;
                view_size += 1;
                switch (current) {
                    case '"':
                        if (!in_escape) {
                            running=false;
                        } else {
                            in_escape = false;
                        }
                        break;
                    case '\\':
                        in_escape = !in_escape;
                        break;
                    default:
                        in_escape = false;
                        break;
                }
            }
            return Token{location, TokenType::StringLiteral, VIEW};
        };
        std::function<Token()> character = [&]() -> Token {
            auto view_start = POSITION;
            auto location = LOCATION;
            std::size_t view_size = 1;
            auto in_escape = false;
            ADVANCE;
            while (true) {
                auto current = PEEK;
                if (END || current == '\n') {
                    if (errorInvalid) {
                        error::raise_exiting_error("LEXER", "unterminated character literal", location,error::ErrorCode::UnterminatedCharacterLiteral);
                    } else {
                        return Token{LOCATION, TokenType::Error, VIEW};
                    }
                }
                ADVANCE;
                view_size += 1;
                switch (current) {
                    case '\'':
                        if (!in_escape) {
                            goto loopEnd;
                        } else {
                            in_escape = false;
                        }
                        break;
                    case '\\':
                        in_escape = !in_escape;
                        break;
                    default:
                        in_escape = false;
                        break;
                }
            }
            loopEnd:
            return Token{location, TokenType::CharacterLiteral, VIEW};
        };
        std::function<std::size_t()> binary = [&]() -> std::size_t {
            std::size_t size = 0;
            while (true) {
                if (!validB(PEEK)) return size;
                ADVANCE;
                ++size;
            }
        };
        std::function<std::size_t()> octal = [&]() -> std::size_t {
            std::size_t size = 0;
            while (true) {
                if (!validO(PEEK)) return size;
                ADVANCE;
                ++size;
            }
        };
        std::function<std::size_t()> hexadecimal = [&]() -> std::size_t {
            std::size_t size = 0;
            while (true) {
                if (!validX(PEEK)) return size;
                ADVANCE;
                ++size;
            }
        };
        std::function<Token()> number = [&]() -> Token {
            auto start_location = LOCATION;
            auto view_start = POSITION;
            std::size_t view_size = 0;
            auto flt = false;
            auto imaginary = false;
            if (PEEK == '0') {
                view_size++;
                ADVANCE;
                auto next = PEEK;
                switch (next) {
                    case 'x':
                    case 'X':
                        view_size++;
                        ADVANCE;
                        view_size += hexadecimal();
                        return Token{start_location, TokenType::HexLiteral, VIEW};
                    case 'o':
                    case 'O':
                        view_size++;
                        ADVANCE;
                        view_size += octal();
                        return Token{start_location, TokenType::OctalLiteral, VIEW};
                    case 'b':
                    case 'B':
                        view_size++;
                        ADVANCE;
                        view_size += binary();
                        return Token{start_location, TokenType::BinaryLiteral, VIEW};
                    default:
                        break;
                }
            }
            while (validDMid(PEEK)) {
                view_size++;
                ADVANCE;
            }
            if (PEEK == '.') {
                flt = true;
                view_size++;
                ADVANCE;
                while (validDMid(PEEK)) {
                    view_size++;
                    ADVANCE;
                }
            }
            if (std::tolower(PEEK) == 'e') {
                flt = true;
                view_size++;
                ADVANCE;
                if (PEEK == '-' || PEEK == '+') {
                    view_size++;
                    ADVANCE;
                }
                while (validDMid(PEEK)) {
                    view_size++;
                    ADVANCE;
                }
            }
            if (PEEK == 'I') {
                imaginary = true;
                view_size++;
                ADVANCE;
            }
            return Token{
                start_location,
                imaginary ? TokenType::ImaginaryLiteral : flt ? TokenType::FloatingLiteral : TokenType::DecimalLiteral,
                VIEW
            };
        };
        std::function<Token()> identifier = [&]() -> Token {
            auto start_location = LOCATION;
            auto view_start = POSITION;
            std::size_t view_size = 0;
            while (validIMid(PEEK)) {
                view_size++;
                ADVANCE;
            }
            if (PEEK == '=') {
                auto x = getKW(buffer.substr(view_start,view_size+1));
                if (x.has_value()) {
                    view_size++;
                    ADVANCE;
                    auto view = VIEW;
                    return Token{start_location, x.value(), view};
                }
            }

            auto view = VIEW;
            auto opt = getKW(view);
            return Token{start_location, validIntegerType(view) ? (view[0] == 'u'? TokenType::UnsignedIntType : TokenType::SignedIntType) : opt.value_or(isMacro(view)?TokenType::BuiltinReference:TokenType::Identifier), view};
        };
        std::function<Token()> builtin = [&]() -> Token {
            auto start_location = LOCATION;
            auto view_start = POSITION;
            std::size_t view_size = 1;
            ADVANCE;
            while (validIMid(PEEK)) {
                view_size++;
                ADVANCE;
            }
            return Token{start_location, view_size > 1? TokenType::BuiltinReference : TokenType::Dereference, VIEW};
        };
        while (!END) {
            auto current = PEEK;
            auto start_location = LOCATION;
            auto view_start = POSITION;
            std::size_t view_size = 0;
            switch (current) {
                case '\n':
                    SINGLE(TokenType::NewLine);
                    break;
                case '\r':
                case '\t':
                case ' ':
                    ADVANCE;
                    break;
                case '=': {
                    ADVANCE;
                    view_size++;
                    auto lookahead = PEEK;
                    switch (lookahead) {
                        case '=':
                            ADVANCE;
                            view_size++;
                            if (PEEK == '>') {
                                SINGLE(TokenType::DoubleThickArrow);
                            } else {
                                ADD(TokenType::EqualTo);
                            }
                            break;
                        case '>':
                            SINGLE(TokenType::ThickArrow);
                            break;
                        default:
                            ADD(TokenType::Assign);
                            break;
                    }
                    break;
                }
                case '/': {
                    ADVANCE;
                    view_size++;
                    auto lookahead = PEEK;
                    switch (lookahead) {
                        case '=':
                            SINGLE(TokenType::DivAssign);
                            break;
                        case '*':
                        {
                            auto comment = skipMultiLineComment(start_location,view_start);
                            if (outputComments) {
                                tokens.push_back(comment);
                            }
                            break;
                        }
                        case '/':
                        {
                            auto comment = skipComment(start_location, view_start);
                            if (outputComments) {
                                tokens.push_back(comment);
                            }
                            break;
                        }
                        default:
                            ADD(TokenType::Slash);
                            break;
                    }
                    break;
                }
                case '-': {
                    ADVANCE;
                    view_size++;
                    auto lookahead = PEEK;
                    switch (lookahead) {
                        case '=':
                            SINGLE(TokenType::SubAssign);
                            break;
                        case '-': {
                            ADVANCE;
                            view_size++;
                            auto view_start_2 = POSITION;
                            auto start_location_2 = LOCATION;
                            auto lookahead2 = PEEK;
                            if (lookahead2 == '>') {
                                SINGLE(TokenType::DoubleArrow);
                            } else {
                                tokens.push_back(Token{
                                    start_location,
                                    TokenType::Dash,
                                    buffer.substr(view_start,1)
                                });
                                tokens.push_back(Token{
                                    start_location_2,
                                    TokenType::Dash,
                                    buffer.substr(view_start_2,1)
                                });
                            }
                            break;
                        }
                        case '>':
                            SINGLE(TokenType::SingleArrow);
                            break;
                        default:
                            ADD(TokenType::Dash);
                            break;
                    }
                    break;
                }
                case '\'':
                    tokens.push_back(character());
                    break;
                case '\"':
                    tokens.push_back(str());
                    break;
                case '.':
                {
                    ADVANCE;
                    view_size++;
                    if (PEEK == '.') {
                        ADVANCE;
                        view_size++;
                        if (PEEK == '.') {
                            SINGLE(TokenType::Dot3);
                        } else {
                            ADD(TokenType::Dot2);
                        }
                    } else if (PEEK == '[') {
                        SINGLE(TokenType::Array);
                    } else if (PEEK == '(') {
                        SINGLE(TokenType::Tuple);
                    } else if (PEEK == '{') {
                        SINGLE(TokenType::Object);
                    } else {
                        ADD(TokenType::Dot);
                    }
                    break;
                }
                case ':':
                    ADVANCE;
                    view_size++;
                    if (PEEK == '=') {
                        SINGLE(TokenType::Redefine);
                    } else if (PEEK == '(') {
                        SINGLE(TokenType::Block);
                    }
                    else
                    {
                        ADD(TokenType::Colon);
                    }

                    break;
                case ',':
                    SINGLE(TokenType::Comma);
                    break;
                case ';':
                    SINGLE(TokenType::Semicolon);
                    break;
                case '*': {
                    ADVANCE;
                    view_size++;
                    switch (PEEK) {
                        case '=':
                            SINGLE(TokenType::MulAssign);
                            break;
                        case '~':
                            SINGLE(TokenType::ConstPointer);
                            break;
                        default:
                            ADD(TokenType::Star);
                            break;
                    }
                    break;
                }
                case '&': {
                    SINGLE(TokenType::Ampersand);
                    break;
                }
                case '@':
                    ADVANCE;
                    view_size++;
                    if (PEEK == '*') {
                        SINGLE(TokenType::DynamicCast);
                    } else {
                        ADD(TokenType::Cast);
                    }
                    break;
                case '{':
                    SINGLE(TokenType::LeftBrace);
                    break;
                case '}':
                    SINGLE(TokenType::RightBrace);
                    break;
                case '(':
                    SINGLE(TokenType::LeftParen);
                    break;
                case ')':
                    SINGLE(TokenType::RightParen);
                    break;
                case '[':
                    SINGLE(TokenType::LeftBracket);
                    break;
                case ']': {
                    ADVANCE;
                    view_size++;
                    switch (PEEK) {
                        case '~':
                            SINGLE(TokenType::ConstArray);
                            break;
                        default:
                            ADD(TokenType::RightBracket);
                            break;
                    }
                    break;
                }
                case '#':
                    SINGLE(TokenType::Hash);
                    break;
                case '%':
                    ADVANCE;
                    view_size++;
                    if (PEEK == '=') {
                        SINGLE(TokenType::ModAssign);
                    } else {
                        ADD(TokenType::Percent);
                    }
                    break;
                case '<':
                    ADVANCE;
                    view_size++;
                    switch (PEEK) {
                        case '=': {
                            ADVANCE;
                            view_size++;
                            if (PEEK == '=') {
                                ADVANCE;
                                view_size++;
                                if (PEEK == '(') {
                                    SINGLE(TokenType::BlockYield);
                                }
                                else {
                                    ADD(TokenType::ReversedDoubleThickArrow);
                                }
                            } else {
                                ADD(TokenType::LessThanEqual);
                            }
                            break;
                        }
                        case '<': {
                            ADVANCE;
                            view_size++;
                            if (PEEK == '=') {
                                SINGLE(TokenType::LeftShiftAssign);
                            } else {
                                ADD(TokenType::LeftShift);
                            }
                            break;
                        }
                        case '-':
                            SINGLE(TokenType::ReversedArrow);
                            break;
                        default:
                            ADD(TokenType::LessThan);
                            break;
                    }
                    break;
                case '>':
                {
                    ADVANCE;
                    view_size++;
                    switch (PEEK) {
                        case '=':
                            SINGLE(TokenType::GreaterThanEqual);
                            break;
                        case '>': {
                            ADVANCE;
                            view_size++;
                            if (PEEK == '=') {
                                SINGLE(TokenType::RightShiftAssign);
                            } else {
                                ADD(TokenType::RightShift);
                            }
                            break;
                        }
                        case '~':
                            SINGLE(TokenType::ConstSlice);
                            break;
                        default:
                            ADD(TokenType::GreaterThan);
                            break;
                    }
                    break;
                }
                case '!': {
                    ADVANCE;
                    view_size++;
                    if (PEEK == '=') {
                        SINGLE(TokenType::NotEqualTo);
                    } else {
                        ADD(TokenType::Exclamation); //At some point figure out what to do with this
                    }
                    break;
                }
                case '+': {
                    ADVANCE;
                    view_size++;
                    if (PEEK == '=') {
                        SINGLE(TokenType::AddAssign);
                    } else {
                        ADD(TokenType::Plus);
                    }
                    break;
                }
                case '$':
                    tokens.push_back(builtin());
                    break;
                case '?':
                    SINGLE(TokenType::Question);
                    break;
                case '|':
                    SINGLE(TokenType::Pipe);
                    break;
                default:
                    if (validIBeg(current)) {
                        tokens.push_back(identifier());
                    } else if (validDBeg(current)) {
                        tokens.push_back(number());
                    } else if (errorInvalid) {
                        error::raise_error("lexer", "unexpected character", start_location, error::ErrorCode::UnexpectedCharacter);
                        ADVANCE;
                    } else {
                        SINGLE(TokenType::Error);
                    }
                    break;
            }
        }
        tokens.push_back(Token{LOCATION, TokenType::EoF, ""});
        return tokens;
    }
#undef VIEW
#undef PEEK
#undef ADVANCE
#undef LINE
#undef COLUMN
#undef END
#undef POSITION
#undef LOCATION
#undef ADD
#undef SINGLE
#define MAP(x) {x, #x}
    using enum TokenType;
    const std::map<TokenType,std::string_view> token_names{
            MAP(SingleArrow),
            MAP(DoubleArrow),
            MAP(ThickArrow),
            MAP(DoubleThickArrow),
            MAP(ReversedDoubleThickArrow),
            MAP(ReversedArrow),
            MAP(LeftBrace),
            MAP(RightBrace),
            MAP(LeftBracket),
            MAP(RightBracket),
            MAP(LeftParen),
            MAP(RightParen),
            MAP(Colon),
            MAP(Comma),
            MAP(Semicolon),
            MAP(Dot),
            MAP(Dot2),
            MAP(Dot3),
            MAP(Hash),
            MAP(Question),
            MAP(ConstPointer),
            MAP(ConstArray),
            MAP(ConstSlice),
            MAP(Cast),
            MAP(Star),
            MAP(Percent),
            MAP(Slash),
            MAP(Plus),
            MAP(Dash),
            MAP(Ampersand),
            MAP(LeftShift),
            MAP(RightShift),
            MAP(GreaterThan),
            MAP(LessThan),
            MAP(EqualTo),
            MAP(NotEqualTo),
            MAP(GreaterThanEqual),
            MAP(LessThanEqual),
            MAP(And),
            MAP(Or),
            MAP(Xor),
            MAP(AndAssign),
            MAP(OrAssign),
            MAP(XorAssign),
            MAP(Assign),
            MAP(MulAssign),
            MAP(ModAssign),
            MAP(DivAssign),
            MAP(AddAssign),
            MAP(SubAssign),
            MAP(LeftShiftAssign),
            MAP(RightShiftAssign),
            MAP(Dereference),
            MAP(Tuple),
            MAP(Object),
            MAP(Array),
            MAP(Exponentiate),
            MAP(ExponentiateAssign),
            MAP(Public),
            MAP(Private),
            MAP(Mutable),
            MAP(NoReturn),
            MAP(Member),
            MAP(Enum),
            MAP(Union),
            MAP(Struct),
            MAP(Extern),
            MAP(Let),
            MAP(Def),
            MAP(Fn),
            MAP(Unpack),
            MAP(Float32),
            MAP(Float64),
            MAP(Complex32),
            MAP(Complex64),
            MAP(Match),
            MAP(If),
            MAP(Elif),
            MAP(Else),
            MAP(While),
            MAP(For),
            MAP(Import),
            MAP(Type),
            MAP(None),
            MAP(Void),
            MAP(Discard),
            MAP(Prototype),
            MAP(Inline),
            MAP(Entry),
            MAP(Comptime),
            MAP(Break),
            MAP(Continue),
            MAP(Any),
            MAP(Asm),
            MAP(ComptimeString),
            MAP(Expression),
            MAP(Expressions),
            MAP(Statement),
            MAP(Statements),
            MAP(ComptimeFloat),
            MAP(ComptimeInt),
            MAP(ComptimeComplex),
            MAP(UnsignedSize),
            MAP(SignedSize),
            MAP(SignedIntType),
            MAP(UnsignedIntType),
            MAP(ImaginaryLiteral),
            MAP(FloatingLiteral),
            MAP(DecimalLiteral),
            MAP(HexLiteral),
            MAP(OctalLiteral),
            MAP(BinaryLiteral),
            MAP(Identifier),
            MAP(BuiltinReference),
            MAP(StringLiteral),
            MAP(CharacterLiteral),
            MAP(SingleLineComment),
            MAP(BlockComment),
            MAP(Underscore),
            MAP(Error),
            MAP(EoF),
            MAP(Then),
            MAP(Do),
            MAP(With),
            MAP(Exclamation),
            MAP(NewLine),
            MAP(Bool),
            MAP(True),
            MAP(False),
            MAP(Not),
            MAP(Block),
            MAP(BlockYield),
            MAP(Pipe),
            MAP(Redefine),
            MAP(Generator),
            MAP(DynamicCast),
            MAP(Is),
            MAP(Opaque),
            MAP(Export),
            MAP(Impl),
            MAP(Interface),
            MAP(Dynamic),
            MAP(Constrain)
    };

#undef MAP
    std::string_view name_of(TokenType t) {
        return token_names.at(t);
    }
    std::string to_stream(const std::vector<Token>& tokens) {
        std::stringstream ss;
        for (Token t : tokens) {
            ss << name_of(t.ty) << ' ';
        }
        std::string s = ss.str();
        if (s.ends_with(" ")) {
            return s.substr(0,s.size()-1);
        } else {
            return s;
        }
    }
}