//
// Created by Lexi Allen on 9/21/2022.
//
#include "parser/parser.h"
#include "error.h"
#include "parser/nodes/terminal_nodes.h"
#include "parser/nodes/single_member_nodes.h"
#include "parser/nodes/other_nodes.h"
#include "NotImplementedException.h"
#include <sstream>
#include <utility>

#ifdef __clang__
#define ALWAYS_INLINE [[clang::always_inline]] inline
#elifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline
#endif

template<typename Base, typename T>
static inline bool instanceof(const T ptr) {
    return std::dynamic_pointer_cast<Base>(ptr).get() != nullptr;
}

namespace cheese::parser {

    using
    enum lexer::TokenType;
    struct parser_state;
    using parse_to_value_type = std::function<NodePtr(parser_state &, lexer::Token)>;
    using parse_to_void_type = std::function<void(
            lexer::Token)>; //These are always going to be lambdas with the context in mind
    struct parser_state {
        std::vector<lexer::Token> &tokens;
        size_t location = 0;
        std::vector<std::tuple<error::ErrorCode, Coordinate, std::string>> all_raised_errors;

        [[nodiscard]] lexer::Token &peek();

        [[nodiscard]] lexer::Token &peek_skip_nl();

        [[nodiscard]] std::optional<NodePtr>
        eat(lexer::TokenType expected_type, const std::string &expected_message, cheese::error::ErrorCode code);

        void eatAny();

        void previous();

        [[nodiscard]] NodePtr
        unexpected(lexer::Token &token, const std::string &expected, cheese::error::ErrorCode code) {
            if (token.ty == EoF) {
                return raise("parser", "unexpected EOF. expected: " + expected, token.location, code);
            } else if (token.ty == StringLiteral || token.ty == CharacterLiteral) {
                return raise("parser", "unexpected token " + std::string(token.value) + ", expected: " + expected,
                             token.location, code);
            } else if (token.ty == NewLine) {
                return raise("parser", "unexpected newline. expected: " + expected, token.location, code);
            } else {
                return raise("parser", "unexpected token '" + std::string(token.value) + "', expected: " + expected,
                             token.location, code);
            }
        }

        [[nodiscard]] NodePtr raise(const char *module, std::string message, Coordinate loc, error::ErrorCode code) {
            all_raised_errors.push_back({code, loc, message});
            error::raise_error(module, message, loc, code);
            return (new nodes::ErrorNode(loc, static_cast<uint64_t>(code), message))->get();
        }

        //Only eats tokens on success, not on default
        inline NodePtr
        eat_switch_value(std::unordered_map<lexer::TokenType, parse_to_value_type> paths, parse_to_value_type def) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                eatAny();
                return paths[token.ty](*this, token);
            }
            //On default we don't eat anything
            return def(*this, token);
        }

        inline NodePtr
        peek_switch_value(std::unordered_map<lexer::TokenType, parse_to_value_type> paths, parse_to_value_type def) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                return paths[token.ty](*this, token);
            }
            return def(*this, token);
        }

        inline NodePtr peek_skip_nl_switch_value(std::unordered_map<lexer::TokenType, parse_to_value_type> paths,
                                                 parse_to_value_type def) {
            auto token = peek_skip_nl();
            if (paths.contains(token.ty)) {
                return paths[token.ty](*this, token);
            }
            return def(*this, token);
        }

        inline NodePtr
        eat_switch_value_err(std::unordered_map<lexer::TokenType, parse_to_value_type> paths, std::string expected,
                             error::ErrorCode errorCode) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                eatAny();
                return paths[token.ty](*this, token);
            }
            //On default we don't eat anything
            return unexpected(token, expected, errorCode);
        }

        inline NodePtr
        peek_switch_value_err(std::unordered_map<lexer::TokenType, parse_to_value_type> paths, std::string expected,
                              error::ErrorCode errorCode) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                return paths[token.ty](*this, token);
            }
            return unexpected(token, expected, errorCode);
        }

        inline NodePtr peek_skip_nl_switch_value_err(std::unordered_map<lexer::TokenType, parse_to_value_type> paths,
                                                     std::string expected, error::ErrorCode errorCode) {
            auto token = peek_skip_nl();
            if (paths.contains(token.ty)) {
                return paths[token.ty](*this, token);
            }
            return unexpected(token, expected, errorCode);
        }

        inline void
        eat_switch_void(std::unordered_map<lexer::TokenType, parse_to_void_type> paths, parse_to_void_type def) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                eatAny();
                paths[token.ty](token);
            }
            //On default we don't eat anything
            def(token);
        }

        inline void
        peek_switch_void(std::unordered_map<lexer::TokenType, parse_to_void_type> paths, parse_to_void_type def) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                return paths[token.ty](token);
            }
            return def(token);
        }

        inline void peek_skip_nl_void(std::unordered_map<lexer::TokenType, parse_to_void_type> paths,
                                      parse_to_void_type def) {
            auto token = peek_skip_nl();
            if (paths.contains(token.ty)) {
                paths[token.ty](token);
            }
            def(token);
        }

        inline std::optional<NodePtr>
        eat_switch_void_err(std::unordered_map<lexer::TokenType, parse_to_void_type> paths, std::string expected,
                            error::ErrorCode errorCode) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                eatAny();
                paths[token.ty](token);
                return std::optional<NodePtr>{};
            }
            //On default we don't eat anything
            return unexpected(token, expected, errorCode);
        }

        inline std::optional<NodePtr>
        peek_switch_void_err(std::unordered_map<lexer::TokenType, parse_to_void_type> paths, std::string expected,
                             error::ErrorCode errorCode) {
            auto token = peek();
            if (paths.contains(token.ty)) {
                paths[token.ty](token);
                return std::optional<NodePtr>{};
            }
            return unexpected(token, expected, errorCode);
        }

        inline std::optional<NodePtr>
        peek_skip_nl_switch_void_err(std::unordered_map<lexer::TokenType, parse_to_void_type> paths,
                                     std::string expected, error::ErrorCode errorCode) {
            auto token = peek_skip_nl();
            if (paths.contains(token.ty)) {
                paths[token.ty](token);
                return std::optional<NodePtr>{};
            }
            return unexpected(token, expected, errorCode);
        }
    };

    template<typename T>
    inline parse_to_void_type wrap_void(T f) {
        return [f](lexer::Token) { f(); };
    }

    inline parse_to_void_type wrap_eat(parser_state &state) {
        return [&](lexer::Token) { state.eatAny(); };
    }

    inline parse_to_value_type wrap_value(std::function<NodePtr(parser_state &)> f) {
        return [f](parser_state &s, lexer::Token) { return f(s); };
    }

    template<class T> NodePtr single_value(parser_state& state, lexer::Token front) {
        auto location = front.location;
        state.eatAny();
        return (new T(location))->get();
    }

    inline parse_to_value_type unexpected(std::string message, error::ErrorCode e, bool eat = true) {
        if (!eat)
            return [&](parser_state &s, lexer::Token t) {
                auto n = s.unexpected(t, message, e);
                return n;
            };
        else {
            return [&](parser_state &s, lexer::Token t) {
                auto n = s.unexpected(t, message, e);
                if (eat) s.eatAny();
                return n;
            };
        }
    }

    lexer::Token &parser_state::peek() {
        return tokens[location];
    }

    void parser_state::previous() {
        if (location > 0) location--;
    }

    lexer::Token &parser_state::peek_skip_nl() {
        while (peek().ty == NewLine) {
            static_cast<void>(eatAny());
        }
        return peek();
    }

    void parser_state::eatAny() {
        if (location < tokens.size() - 1) location++;
    }

    std::optional<NodePtr> parser_state::eat(lexer::TokenType expected_type, const std::string &expected_message,
                                             cheese::error::ErrorCode code) {
        auto &token = peek();
        if (token.ty != expected_type) {
            return unexpected(token, expected_message, code);
        } else {
            eatAny();
            return std::optional<NodePtr>{};
        }
    }

    NodePtr parse_program(parser_state &state);

    NodePtr parse(std::vector<lexer::Token> &tokens) {
        auto location = tokens[0].location;
        parser_state state{tokens, 0};
        auto program = parse_program(state);
        if (state.all_raised_errors.size() == 1) {
            auto first = state.all_raised_errors[0];
            error::raise_exiting_error("parser", std::get<2>(first), std::get<1>(first), std::get<0>(first));
        } else if (state.all_raised_errors.size() > 1) {
            error::raise_exiting_error("parser", "multiple syntax errors detected", location,
                                       error::ErrorCode::GeneralCompilerError);
        }
        return program;
    }

    NodePtr parse_statement(parser_state &state);

    NodePtr parse_program(parser_state &state) {
        NodeList interfaces{};
        NodeList children{};
        auto &front = state.peek_skip_nl();
        auto location = front.location;
        bool last_was_field = false;
        while (front.ty != EoF) {
            state.peek_switch_void(
                    {
                            {Comma,
                                    [&](lexer::Token) {
                                        if (!last_was_field) {
                                            children.push_back(state.raise("parser",
                                                                           "Expected a semicolon to separate 2 non-field declarations",
                                                                           front.location,
                                                                           error::ErrorCode::IncorrectSeparator));
                                            state.eatAny();
                                        } else {
                                            state.eatAny();
                                        }
                                    }},
                            {Semicolon,
                                    wrap_eat(state)}
                    },
                    [&](lexer::Token) {
                        auto last = parse_statement(state);
                        children.push_back(last);
                        if (instanceof<nodes::Field>(last)) {
                            last_was_field = true;
                        } else {
                            last_was_field = false;
                        }
                    }
            );
            front = state.peek_skip_nl();
        }
        return (new nodes::Structure(location, interfaces, children, false))->get();
    }

    NodePtr parse_import(parser_state &state);

    NodePtr parse_function(parser_state &state);

    NodePtr parse_generator(parser_state &state);

    NodePtr parse_variable_decl(parser_state &state);

    NodePtr parse_field(parser_state &state);

    NodePtr parse_definition(parser_state &state);

    NodePtr parse_comptime(parser_state &state);

    NodePtr parse_expression(parser_state &state);

    NodePtr parse_unnamed_field(parser_state &state);

    NodePtr parse_block_statement(parser_state &state);

    NodePtr parse_mixin(parser_state &state);

    NodePtr parse_operator(parser_state &state);

    NodePtr parse_statement(parser_state &state) {
//        auto &front = state.peek_skip_nl()
        return state.peek_skip_nl_switch_value({
                                                       {Import,     wrap_value(parse_import)},
                                                       {Fn,         wrap_value(parse_function)},
                                                       {Generator,  wrap_value(parse_generator)},
                                                       {Let,        wrap_value(parse_variable_decl)},
                                                       {Identifier, wrap_value(parse_field)},
                                                       {Underscore, wrap_value(parse_unnamed_field)},
                                                       {Def,        wrap_value(parse_definition)},
                                                       {Comptime,   wrap_value(parse_comptime)},
                                                       {Impl,       wrap_value(parse_mixin)},
                                                       {Operator,wrap_value(parse_operator)}
                                               }, unexpected("a structure statement (i.e. an import, function declaration, variable declaration, variable definition, comptime, or a field)",
                                                             error::ErrorCode::ExpectedStructureStatement)
        );
    }

    NodePtr parse_import(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        //Get the actual import path
        std::string path;
        std::string resolved_name;
        std::string last_name;
        while (true) {
            auto front = state.peek_skip_nl();
            if (front.ty != Dot && front.ty != Dereference && front.ty != Identifier) {
                break;
            } else if (front.ty == Dot) {
                path += "/";
            } else if (front.ty == Dereference) {
                path += "..";
            } else if (front.ty == Identifier) {
                path += front.value;
                last_name = front.value;
            }
            state.eatAny();
        }
        if (path.empty()) {
            return state.raise("parser", "Missing import path following import statement", location,
                               error::ErrorCode::ExpectedImportPath);
        }
        auto front = state.peek_skip_nl();
        if (front.ty == Cast) {
            state.eatAny();
            front = state.peek_skip_nl();
            resolved_name = front.value;
            auto ident = state.eat(Identifier, "an identifier following an '@' for an import statement",
                                   error::ErrorCode::ExpectedImportName);
            if (ident.has_value()) {
                return ident.value();
            }
        } else {
            resolved_name = last_name;
        }
        return (new nodes::Import(location, path, resolved_name))->get();
    }

    NodePtr parse_argument(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        if (front.ty == Semicolon) {
            return state.raise("parser", "expected ',' as a separator, not ';'", front.location,
                               error::ErrorCode::IncorrectSeparator);
        }
        if (front.ty == Self) {
            state.eatAny();
            return (new nodes::Self(location))->get();
        } else if (front.ty == ConstSelf) {
            state.eatAny();
            return (new nodes::ConstSelf(location))->get();
        }
        if (front.ty != Underscore && front.ty != Identifier) {
            return state.unexpected(front, "'_' or IDENT", error::ErrorCode::ExpectedName);
        }
        std::optional<std::string> name;
        if (front.ty == Identifier) {
            name = static_cast<std::string>(front.value);
        }
        state.eatAny();
        auto colon = state.eat(Colon, "a ':' to denote the argument type", error::ErrorCode::ExpectedColon);
        if (colon.has_value()) {
            return colon.value();
        }
        auto type = parse_expression(state);
        bool comptime = state.peek_skip_nl().ty == Comptime;
        if (comptime) {
            state.eatAny();
        }
        return (new nodes::Argument(location, name, type, comptime))->get();
    }

    bool valid_flag(lexer::TokenType t) {
        return t == Inline || t == Extern || t == Export || t == Comptime || t == Public || t == Private ||
               t == Mutable || t == Entry;
    }

    FlagSet parse_flags(parser_state &state) {
        FlagSet f{false, false, false, false, false, false, false, false};
        auto front = state.peek_skip_nl();
        while (valid_flag(front.ty)) {
            switch (front.ty) {
                case Inline:
                    f.inlin = true;
                    break;
                case Extern:
                    f.exter = true;
                    break;
                case Export:
                    f.exp = true;
                    break;
                case Comptime:
                    f.comptime = true;
                    break;
                case Public:
                    f.pub = true;
                    break;
                case Private:
                    f.priv = true;
                    break;
                case Mutable:
                    f.mut = true;
                    break;
                case Entry:
                    f.entry = true;
                    break;
                default:
                    //Maybe panic here
                    break;
            }
            state.eatAny();
            front = state.peek_skip_nl();
        }


        return f;
    }

    NodePtr parse_function(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        //At the top level we don't expect functional types or the such here
        auto name = state.peek_skip_nl().value;
        static_cast<void>(state.eat(Identifier, "a function name for a structure level 'function'",
                                    error::ErrorCode::ExpectedFunctionGeneratorName));
        NodeList arguments{};
        auto front = state.peek_skip_nl();
        while (front.ty != ThickArrow && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                auto argument = parse_argument(state);
                if (instanceof<nodes::ErrorNode>(argument)) {
                    //Continue until the next argument begins
                    front = state.peek_skip_nl();
                    while (front.ty != ThickArrow && front.ty != Identifier && front.ty != Underscore &&
                           front.ty != EoF) {
                        state.eatAny();
                        front = state.peek_skip_nl();
                    }
                }
                arguments.push_back(argument);
            }
            front = state.peek_skip_nl();
        }
        static_cast<void>(state.eat(ThickArrow, "a '=>' to denote the return type",
                                    error::ErrorCode::ExpectedReturnSpecifier));
        NodePtr return_type = parse_expression(state);
        if (instanceof<nodes::ObjectCall>(return_type)) {
            error::make_note("parser",
                             "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a function, if this is not what you want this can be fixed by inserting  newline before the '{'",
                             return_type->location);
        }
        FlagSet fn_flags = parse_flags(state);
        if (state.peek_skip_nl().ty == Prototype) {
            state.eatAny();
            return (new nodes::FunctionPrototype(location, static_cast<std::string>(name), arguments, return_type,
                                                 fn_flags))->get();
        } else if (state.peek_skip_nl().ty == Import) {
            state.eatAny();
            return (new nodes::FunctionImport(location, static_cast<std::string>(name), arguments, return_type,
                                              fn_flags))->get();
        } else {
            auto body = parse_block_statement(state);
            return (new nodes::Function(location, static_cast<std::string>(name), arguments, return_type, fn_flags,
                                        body))->get();
        }
    }

    NodePtr parse_generator(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        //At the top level we don't expect functional types or the such here
        auto name = state.peek_skip_nl().value;
        static_cast<void>(state.eat(Identifier, "a generator name for a structure level 'generator'",
                                    error::ErrorCode::ExpectedFunctionGeneratorName));
        NodeList arguments{};
        auto front = state.peek_skip_nl();
        while (front.ty != ThickArrow && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                auto argument = parse_argument(state);
                if (instanceof<nodes::ErrorNode>(argument)) {
                    //Continue until the next argument begins
                    front = state.peek_skip_nl();
                    while (front.ty != ThickArrow && front.ty != Identifier && front.ty != Underscore &&
                           front.ty != EoF) {
                        state.eatAny();
                        front = state.peek_skip_nl();
                    }
                }
                arguments.push_back(argument);
            }
            front = state.peek_skip_nl();
        }
        static_cast<void>(state.eat(ThickArrow, "a '=>' to denote the return type",
                                    error::ErrorCode::ExpectedReturnSpecifier));
        NodePtr return_type = parse_expression(state);
        if (instanceof<nodes::ObjectCall>(return_type)) {
            error::make_note("parser",
                             "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a generator, if this is not what you want this can be fixed by inserting  newline before the '{'",
                             return_type->location);
        }
        FlagSet fn_flags = parse_flags(state);
        if (state.peek_skip_nl().ty == Prototype) {
            state.eatAny();
            return (new nodes::GeneratorPrototype(location, static_cast<std::string>(name), arguments, return_type,
                                                  fn_flags))->get();
        } else if (state.peek_skip_nl().ty == Import) {
            state.eatAny();
            return (new nodes::GeneratorImport(location, static_cast<std::string>(name), arguments, return_type,
                                               fn_flags))->get();
        } else {
            auto body = parse_block_statement(state);
            return (new nodes::Generator(location, static_cast<std::string>(name), arguments, return_type, fn_flags,
                                         body))->get();
        }
    }

    NodePtr parse_def(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        auto name = front.value;
        static_cast<void>(state.eat(Identifier, "a variable name", error::ErrorCode::ExpectedName));
        front = state.peek_skip_nl();
        std::optional<NodePtr> ty;
        if (front.ty == Colon) {
            state.eatAny();
            ty = parse_expression(state);
        }
        auto flags = parse_flags(state);
//        if (err.has_value()) {
//            return err.value(); //Parse everything else first as it means there is a higher chance of recovery
//        }
        return (new nodes::VariableDefinition(location, static_cast<std::string>(name), ty, flags))->get();
    }

    NodePtr parse_definition(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto def = parse_def(state);
        def->location = location;
        return def;
    }

    NodePtr parse_destructure_statement(parser_state &state);

    NodePtr parse_destructure_array(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeList children{};
        while (front.ty != RightBracket && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                children.push_back(parse_destructure_statement(state));
            }
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightBracket, "a ']' to close off an array destructuring",
                             error::ErrorCode::ExpectedClosingBracket);
        if (err.has_value()) {
            children.push_back(err.value());
        }
        return (new nodes::ArrayDestructure(location, children))->get();
    }

    NodePtr parse_destructure_tuple(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeList children{};
        while (front.ty != RightParen && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                children.push_back(parse_destructure_statement(state));
            }
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightParen, "a ')' to close off a tuple destructuring",
                             error::ErrorCode::ExpectedClosingParentheses);
        if (err.has_value()) {
            children.push_back(err.value());
        }
        return (new nodes::TupleDestructure(location, children))->get();
    }

    NodePtr parse_destructure_structure(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeDict children;
        std::uint64_t error_num = 0;
        while (front.ty != RightBrace && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else if (front.ty == Underscore || front.ty == Identifier) {
                auto name = static_cast<std::string>(front.value);
                state.eatAny();
                auto peek = state.peek_skip_nl();
                std::optional<NodePtr> err;
                if (peek.ty == Block) {
                    err = state.raise("parser",
                                      "':(' is lexed as the beginning to a name block rather than ':' and '(' separately, to fix this add whitespace between ':' and '(' ",
                                      peek.location, error::ErrorCode::AccidentalNamedBlock);
                } else {
                    err = state.eat(Colon, "a ':'", error::ErrorCode::ExpectedColon);
                }
                if (err.has_value()) {
                    children["$e" + std::to_string(error_num++)] = err.value();
                    continue;
                }
                auto value = parse_destructure_statement(state);
                children[name] = value;
            } else {
                children["$e" + std::to_string(error_num++)] = state.unexpected(front,
                                                                                "a structure destructuring specifier",
                                                                                error::ErrorCode::ExpectedDestructuringStatement);
                state.eatAny();
            }
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightBrace, "a '}' to close off a destructuring statement",
                             error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            children["$e" + std::to_string(error_num)] = err.value();
        }
        return (new nodes::StructureDestructure(location, children))->get();
    }

    NodePtr parse_destructure_statement(parser_state &state) {
        auto front = state.peek_skip_nl();
        switch (front.ty) {
            case Underscore:
                state.eatAny();
                return (new nodes::Underscore(front.location))->get();
            case LeftBracket:
                return parse_destructure_array(state);
            case LeftParen:
                return parse_destructure_tuple(state);
            case LeftBrace:
                return parse_destructure_structure(state);
            default:
                return parse_def(state);
        }
    }

    NodePtr parse_destructure(parser_state &state, Coordinate location) {
        auto structure = parse_destructure_statement(state);
        auto err = state.eat(Assign, "an '=' for a variable destructuring", error::ErrorCode::UninitializedDeclaration);
        NodePtr value;
        if (err.has_value()) {
            value = err.value();
        } else {
            value = parse_expression(state);
        }
        return (new nodes::Destructure(location, structure, value))->get();
    }

    NodePtr parse_variable_decl(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto fty = state.peek_skip_nl().ty;
        if (fty == LeftBrace || fty == LeftBracket || fty == LeftParen || fty == Underscore) {
            return parse_destructure(state, location);
        }

        auto def = parse_def(state);
        auto err = state.eat(Assign, "an '=' for a variable declaration", error::ErrorCode::UninitializedDeclaration);
        NodePtr val;
        if (err.has_value()) {
            val = err.value();
        } else {
            val = parse_expression(state);
        }
        return (new nodes::VariableDeclaration(location, def, val))->get();
    }

    NodePtr parse_field(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        std::optional<std::string> name = static_cast<std::string>(state.peek_skip_nl().value);
        state.eatAny();
        auto front = state.peek_skip_nl();
        std::optional<NodePtr> err;
        if (front.ty == Block) {
            err = state.raise("parser",
                              "':(' is lexed as the beginning to a name block rather than ':' and '(' separately, to fix this add whitespace between ':' and '(' ",
                              front.location, error::ErrorCode::AccidentalNamedBlock);
        } else {
            err = state.eat(Colon, "a ':' for a field declaration", error::ErrorCode::ExpectedColon);
        }
        NodePtr type;
        if (err.has_value()) {
            type = err.value();
        } else {
            type = parse_expression(state);
        }
        auto flags = parse_flags(state);
        return (new nodes::Field(location, name, type, flags))->get();
    }

    NodePtr parse_unnamed_field(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        std::optional<std::string> name;
        state.eatAny();
        auto front = state.peek_skip_nl();
        std::optional<NodePtr> err;
        if (front.ty == Block) {
            err = state.raise("parser",
                              "':(' is lexed as the beginning to a name block rather than ':' and '(' separately, to fix this add whitespace between ':' and '(' ",
                              front.location, error::ErrorCode::AccidentalNamedBlock);
        } else {
            err = state.eat(Colon, "a ':' for a field declaration", error::ErrorCode::ExpectedColon);
        }
        NodePtr type;
        FlagSet flags;
        if (err.has_value()) {
            type = err.value();
        } else {
            type = parse_expression(state);
            flags = parse_flags(state);
        }
        return (new nodes::Field(location, name, type, flags))->get();
    }


    NodePtr parse_comptime(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        return (new nodes::Comptime(location, parse_expression(state)))->get();
    }

    NodePtr parse_mixin(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto structure = parse_expression(state);
        NodeList interfaces{};
        NodeList arguments{};
        NodeList children{};
        std::optional<NodePtr> err;
        auto front = state.peek_skip_nl();
        if (front.ty == DoubleColon) {
            state.eatAny();
            while (front.ty != Colon && front.ty != EoF) {
                if (front.ty == Comma) {
                    state.eatAny();
                } else {
                    auto argument = parse_argument(state);
                    if (instanceof<nodes::ErrorNode>(argument)) {
                        //Continue until the next argument begins
                        front = state.peek_skip_nl();
                        while (front.ty != ThickArrow && front.ty != Identifier && front.ty != Underscore &&
                               front.ty != EoF) {
                            state.eatAny();
                            front = state.peek_skip_nl();
                        }
                    }
                    arguments.push_back(argument);
                }
                front = state.peek_skip_nl();
            }
        }

        if (front.ty == Block) {
            err = state.raise("parser",
                              "':(' is lexed as the beginning to a name block rather than ':' and '(' separately, to fix this add whitespace between ':' and '(' ",
                              front.location, error::ErrorCode::AccidentalNamedBlock);
        } else {
            err = state.eat(Colon, "a ':' to begin a mixin", error::ErrorCode::ExpectedColon);
        }
        if (err.has_value()) {
            interfaces.push_back(err.value());
        }
        front = state.peek_skip_nl();
        while (front.ty != EoF && front.ty != LeftBrace) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                auto expr = parse_expression(state);
                if (instanceof<nodes::ObjectCall>(expr)) {
                    error::make_note("parser",
                                     "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a mixin, if this is not what you want this can be fixed by inserting a comma or newline before the '{'",
                                     expr->location);
                }
                interfaces.push_back(expr);
            }
            front = state.peek_skip_nl();
        }
        err = state.eat(LeftBrace, "a '{' to begin the body of a mixin", error::ErrorCode::ExpectedOpeningBrace);
        if (err.has_value()) {
            children.push_back(err.value());
            //Return an empty mixin
            return (new nodes::Mixin(location, structure, arguments, interfaces, children))->get();
        }

        front = state.peek_skip_nl();
        bool last_was_field = false;
        while (front.ty != EoF && front.ty != RightBrace) {
            if (front.ty == Comma) {
                if (!last_was_field) {
                    children.push_back(
                            state.raise("parser", "Expected a semicolon to separate 2 non-field declarations",
                                        front.location, error::ErrorCode::IncorrectSeparator));
                    state.eatAny();
                } else {
                    state.eatAny();
                }
            } else if (front.ty == Semicolon) {
                state.eatAny();
            } else {

                auto last = parse_statement(state);
                children.push_back(last);
                if (instanceof<nodes::Field>(last)) {
                    last_was_field = true;
                } else {
                    last_was_field = false;
                }
            }
            front = state.peek_skip_nl();
        }
        err = state.eat(RightBrace, "a '}' to end a structure declaration", error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            children.push_back(err.value());
        }
        return (new nodes::Mixin(location, structure, arguments, interfaces, children))->get();
    }

    NodePtr parse_primary(parser_state &state, std::optional<NodePtr> lookbehind_base = std::optional<NodePtr>{});



    template<class T> NodePtr unary_expression(parser_state& state, lexer::Token(front)) {
        auto location = front.location;
        state.eatAny();
        auto child = parse_primary(state);
        return (new T(location,child))->get();
    }

    NodePtr parse_expression(parser_state &state, NodePtr old_lhs, uint8_t min_precedence);

    bool is_binary_operation(lexer::TokenType ty) {
        switch (ty) {
            case Star: //Multiplication
            case Slash: //Division
            case Ampersand: //Combination
            case Percent: //Modulation
            case Plus: //Addition
            case Dash: //Subtraction
            case EqualTo:
            case NotEqualTo:
            case GreaterThanEqual:
            case GreaterThan:
            case LessThan:
            case LessThanEqual:
            case LeftShift:
            case RightShift:
            case Xor:
            case Or:
            case And:
            case Cast: //After this will come dynamic cast and such once I get to it
            case DynamicCast:
            case Is:
            case Dot2:
                return true;
            default:
                return false;
        }
    }

    uint8_t precedence(lexer::TokenType ty) {
        switch (ty) {
            case Star:
            case Slash:
            case Percent:
                return 10;
            case Plus:
            case Dash:
                return 9;
            case LeftShift:
            case RightShift:
                return 8;
            case LessThanEqual:
            case LessThan:
            case GreaterThan:
            case GreaterThanEqual:
                return 7;
            case EqualTo:
            case NotEqualTo:
            case DynamicCast:
            case Cast:
            case Is:
                return 6;
            case And:
                return 5;
            case Xor:
                return 4;
            case Or:
                return 3;
            case Ampersand:
                return 2;
            case Dot2:
                return 1;
            default:
                return 0;
        }
    }

#define RETURN_NODE(N) return (new nodes:: N (tok.location,lhs,rhs))->get();

    NodePtr create_node_from_binary_operator(lexer::Token tok, const NodePtr &lhs, const NodePtr &rhs) {
        switch (tok.ty) {
            case Star:
                RETURN_NODE(Multiplication)
            case Slash:
                RETURN_NODE(Division)
            case Percent:
                RETURN_NODE(Modulus)
            case Plus:
                RETURN_NODE(Addition)
            case Dash:
                RETURN_NODE(Subtraction)
            case LeftShift:
                RETURN_NODE(LeftShift)
            case RightShift:
                RETURN_NODE(RightShift)
            case LessThan:
                RETURN_NODE(LesserThan)
            case GreaterThan:
                RETURN_NODE(GreaterThan)
            case LessThanEqual:
                RETURN_NODE(LesserEqual)
            case GreaterThanEqual:
                RETURN_NODE(GreaterEqual)
            case EqualTo:
                RETURN_NODE(EqualTo)
            case NotEqualTo:
                RETURN_NODE(NotEqualTo)
            case And:
                RETURN_NODE(And)
            case Or:
                RETURN_NODE(Or)
            case Xor:
                RETURN_NODE(Xor)
            case Ampersand:
                RETURN_NODE(Combination)
            case Redefine:
                RETURN_NODE(Reassignment)
            case Assign:
                RETURN_NODE(Assignment)
            case AddAssign:
                RETURN_NODE(SumAssignment)
            case SubAssign:
                RETURN_NODE(DifferenceAssignment)
            case MulAssign:
                RETURN_NODE(MultiplicationAssignment)
            case DivAssign:
                RETURN_NODE(DivisionAssignment)
            case ModAssign:
                RETURN_NODE(ModulusAssignment)
            case LeftShiftAssign:
                RETURN_NODE(LeftShiftAssignment)
            case RightShiftAssign:
                RETURN_NODE(RightShiftAssignment)
            case AndAssign:
                RETURN_NODE(AndAssignment)
            case OrAssign:
                RETURN_NODE(OrAssignment)
            case XorAssign:
                RETURN_NODE(XorAssignment)
            case Is:
                RETURN_NODE(IsType)
            case DynamicCast:
                RETURN_NODE(DynamicCast)
            case Cast:
                RETURN_NODE(Cast)
            case Dot2:
                RETURN_NODE(Range)
            default:
                return nullptr;
        }
    }

#undef RETURN_NODE

    NodePtr parse_expression(parser_state &state) {
        return parse_expression(state, parse_primary(state), 0);
    }

    NodePtr parse_expression(parser_state &state, NodePtr old_lhs, uint8_t min_precedence) {
        auto lookahead = state.peek();
        auto lhs = std::move(old_lhs);
        while (is_binary_operation(lookahead.ty) && precedence(lookahead.ty) >= min_precedence) {
            auto op = lookahead;
            auto op_prec = precedence(op.ty);
            state.eatAny();
            auto rhs = parse_primary(state);
            lookahead = state.peek();
            while (is_binary_operation(lookahead.ty) && precedence(lookahead.ty) > op_prec) {
                rhs = parse_expression(state, rhs, min_precedence + 1);
                lookahead = state.peek();
            }
            lhs = create_node_from_binary_operator(op, lhs, rhs);
        }
        return lhs;
    }

    NodePtr parse_primary_base(parser_state &state);

    NodeList parse_call_list(parser_state &state, lexer::TokenType ending_type) {
        NodeList list;
        while (state.peek_skip_nl().ty != ending_type && state.peek_skip_nl().ty != EoF) {
            if (state.peek_skip_nl().ty == Comma) {
                state.eatAny();
                continue;
            }
            list.push_back(parse_expression(state));
        }
        auto err = state.eat(ending_type, "a matching bracket to close a call list (either ']' or ')')",
                             error::ErrorCode::ExpectedClose);
        if (err.has_value()) {
            list.push_back(err.value());
        }
        return list;
    }

    NodeList parse_object_list(parser_state &state) {
        NodeList object_list;
        std::uint64_t err_num{0};
        while (state.peek_skip_nl().ty != RightBrace && state.peek_skip_nl().ty != EoF) {
            if (state.peek_skip_nl().ty == Comma) {
                state.eatAny();
                continue;
            }
            std::string name = static_cast<std::string>(state.peek_skip_nl().value);
            auto err = state.eat(Identifier, "a field name for an object", error::ErrorCode::ExpectedName);
            if (err.has_value()) {
                object_list.push_back((new nodes::FieldLiteral(state.peek_skip_nl().location,"$e" + std::to_string(err_num++),err.value()))->get());
            }
            auto front = state.peek_skip_nl();
            if (front.ty == Block) {
                err = state.raise("parser",
                                  "':(' is lexed as the beginning to a name block rather than ':' and '(' separately, to fix this add whitespace between ':' and '(' ",
                                  front.location, error::ErrorCode::AccidentalNamedBlock);
            } else {
                err = state.eat(Colon, "a ':' for an object", error::ErrorCode::ExpectedColon);
            }
            if (err.has_value()) {
                object_list.push_back((new nodes::FieldLiteral(state.peek_skip_nl().location,"$e" + std::to_string(err_num++),err.value()))->get());
            }
            NodePtr value = parse_expression(state);
            object_list.push_back((new nodes::FieldLiteral(front.location,name,value))->get());
        }
        auto err = state.eat(RightBrace, "a '}' to close off an object literal",
                             error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            object_list.push_back((new nodes::FieldLiteral(state.peek_skip_nl().location,"$e" + std::to_string(err_num++),err.value()))->get());
        }
        return object_list;
    }

    NodePtr parse_primary(parser_state &state, std::optional<NodePtr> lookbehind) {
        auto base = lookbehind.value_or(parse_primary_base(state));
        while (true) {
            auto front = state.peek();
            auto should_break = false;
            switch (front.ty) {
                case Dot: {
                    state.eatAny();
                    auto next = parse_primary_base(state);
                    base = (new nodes::Subscription(front.location,base,next))->get();
                    break;
                }
                case LeftParen: {
                    auto location = front.location;
                    state.eatAny();
                    auto list = parse_call_list(state, RightParen);
                    base = (new nodes::TupleCall(location, base, list))->get();
                    break;
                }
                case LeftBracket: {
                    auto location = front.location;
                    state.eatAny();
                    auto list = parse_call_list(state, RightBracket);
                    base = (new nodes::ArrayCall(location, base, list))->get();
                    break;
                }
                case LeftBrace: {
                    auto location = front.location;
                    state.eatAny();
                    auto nodes = parse_object_list(state);
                    base = (new nodes::ObjectCall(location, base, nodes))->get();
                    break;
                }
                case Dereference: {
                    auto location = front.location;
                    state.eatAny();
                    base = (new nodes::Dereference(location, base))->get();
                    break;
                }
                default:
                    should_break = true;
            }
            if (should_break) break;
        }
        return base;
    }

    NodePtr parse_integer(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        state.eatAny();
        return (new nodes::IntegerLiteral(location, math::BigInteger(front.value)))->get();
    }


    std::string remove_all_sugar(std::string_view str) {
        std::string result;
        for (auto chr: str) {
            if (chr == '_' || chr == 'I') continue;
            result += chr;
        }
        return result;
    }


    NodePtr parse_float(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto value = std::stod(remove_all_sugar(front.value));
        state.eatAny();
        return (new nodes::FloatLiteral(front.location, value))->get();
    }

    NodePtr parse_imaginary(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto value = std::stod(remove_all_sugar(front.value));
        state.eatAny();
        return (new nodes::ImaginaryLiteral(front.location, value))->get();
    }

    NodePtr parse_capture(parser_state &state, bool allow_implicit = true) {
        int capture_type; //0 == '=', 1 == '*', 2 == '*~'
        auto front = state.peek_skip_nl();
        auto location = front.location;
        if (front.ty == Assign) {
            capture_type = 0;
            state.eatAny();
        } else if (front.ty == Star) {
            capture_type = 1;
            state.eatAny();
        } else if (front.ty == ConstPointer) {
            capture_type = 2;
            state.eatAny();
        } else if (front.ty == Identifier || front.ty == Underscore) {
            capture_type = 0;
        } else {
            return state.unexpected(front, "'*', '*~', '=', or an identifier for a capture",
                                    error::ErrorCode::ExpectedCaptureSpecifier);
        }
        front = state.peek_skip_nl();
        if (front.ty == Identifier || front.ty == Underscore) {
            auto name = static_cast<std::string>(front.value);
            state.eatAny();
            if (capture_type == 0) {
                return (new nodes::CopyCapture(location, name))->get();
            } else if (capture_type == 1) {
                return (new nodes::RefCapture(location, name))->get();
            } else {
                return (new nodes::ConstRefCapture(location, name))->get();
            }
        } else if (allow_implicit) {
            if (capture_type == 0) {
                return (new nodes::CopyImplicitCapture(location))->get();
            } else if (capture_type == 1) {
                return (new nodes::RefImplicitCapture(location))->get();
            } else {
                return (new nodes::ConstRefImplicitCapture(location))->get();
            }
        } else {
            return state.unexpected(front, "A name for a capture in a context disallowing implicit captures",
                                    error::ErrorCode::ExpectedName);
        }

    }

    NodePtr parse_closure(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeList arguments;
        while (front.ty != Pipe && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                arguments.push_back(parse_argument(state));
            }
            front = state.peek_skip_nl();
        }
        auto err = state.eat(Pipe, "'|' to end an argument list", error::ErrorCode::UnexpectedEoF);
        if (err.has_value()) {
            arguments.push_back(err.value());
        }
        front = state.peek_skip_nl();
        NodeList captures;
        if (front.ty == LeftBracket) {
            state.eatAny();
            front = state.peek_skip_nl();
            while (front.ty != RightBracket && front.ty != EoF) {
                if (front.ty == Comma) {
                    state.eatAny();
                } else {
                    captures.push_back(parse_capture(state));
                }
                front = state.peek_skip_nl();
            }
            err = state.eat(RightBracket, "']' to end a capture list", error::ErrorCode::UnexpectedEoF);
            if (err.has_value()) {
                captures.push_back(err.value());
            }
        }
        std::optional<NodePtr> return_type;
        if (front.ty == ThickArrow) {
            state.eatAny();
            return_type = parse_expression(state);
            if (instanceof<nodes::ObjectCall>(return_type.value())) {
                error::make_note("parser",
                                 "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a closure, if this is not what you want this can be fixed by inserting  newline before the '{'",
                                 return_type.value()->location);
            }
        }
        NodePtr body = parse_block_statement(state);
        return (new nodes::Closure(location, arguments, captures, return_type, body))->get();
    }

    NodePtr parse_function_type(parser_state &state, Coordinate location) {
        auto arg_types = parse_call_list(state, RightParen);
        static_cast<void>(state.eat(ThickArrow, "a '=>' for signalling the return type of a function type",
                                    error::ErrorCode::ExpectedReturnSpecifier));
        NodePtr return_type = parse_expression(state);
        FlagSet fn_flags = parse_flags(state);
        return (new nodes::FunctionType(location, arg_types, return_type, fn_flags))->get();
    }

    NodePtr parse_anon_function_or_type(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        if (state.peek_skip_nl().ty == LeftParen) {
            state.eatAny();
            return parse_function_type(state, location);
        } else {
            NodeList arguments{};
            auto front = state.peek_skip_nl();
            while (front.ty != ThickArrow && front.ty != EoF) {
                if (front.ty == Comma) {
                    state.eatAny();
                } else {
                    auto argument = parse_argument(state);
                    if (instanceof<nodes::ErrorNode>(argument)) {
                        //Continue until the next argument begins
                        front = state.peek_skip_nl();
                        while (front.ty != ThickArrow && front.ty != Identifier && front.ty != Underscore &&
                               front.ty != EoF) {
                            state.eatAny();
                            front = state.peek_skip_nl();
                        }
                    }
                    arguments.push_back(argument);
                }
                front = state.peek_skip_nl();
            }
            static_cast<void>(state.eat(ThickArrow, "a '=>' to specify a return",
                                        error::ErrorCode::ExpectedReturnSpecifier));
            NodePtr return_type = parse_expression(state);
            if (instanceof<nodes::ObjectCall>(return_type)) {
                error::make_note("parser",
                                 "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a function, if this is not what you want this can be fixed by inserting  newline before the '{'",
                                 return_type->location);
            }
            FlagSet fn_flags = parse_flags(state);
            NodePtr body = parse_block_statement(state);
            return (new nodes::AnonymousFunction(location, arguments, return_type, fn_flags, body))->get();
        }
    }

    NodePtr parse_generator_type(parser_state &state, Coordinate location) {
        auto arg_types = parse_call_list(state, RightParen);
        static_cast<void>(state.eat(ThickArrow, "a '=>' for signalling the return type of a generator type",
                                    error::ErrorCode::ExpectedReturnSpecifier));
        NodePtr return_type = parse_expression(state);
        FlagSet fn_flags = parse_flags(state);
        return (new nodes::GeneratorType(location, arg_types, return_type, fn_flags))->get();
    }

    NodePtr parse_anon_generator_or_type(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        if (state.peek_skip_nl().ty == LeftParen) {
            state.eatAny();
            return parse_generator_type(state, location);
        } else {
            NodeList arguments{};
            auto front = state.peek_skip_nl();

            while (front.ty != ThickArrow && front.ty != EoF) {
                if (front.ty == Comma) {
                    state.eatAny();
                } else {
                    auto argument = parse_argument(state);
                    if (instanceof<nodes::ErrorNode>(argument)) {
                        //Continue until the next argument begins
                        front = state.peek_skip_nl();
                        while (front.ty != ThickArrow && front.ty != Identifier && front.ty != Underscore &&
                               front.ty != EoF) {
                            state.eatAny();
                            front = state.peek_skip_nl();
                        }
                    }
                    arguments.push_back(argument);
                }
                front = state.peek_skip_nl();
            }
            static_cast<void>(state.eat(ThickArrow, "a '=>' to specify a return",
                                        error::ErrorCode::ExpectedReturnSpecifier));
            NodePtr return_type = parse_expression(state);
            if (instanceof<nodes::ObjectCall>(return_type)) {
                error::make_note("parser",
                                 "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a function, if this is not what you want this can be fixed by inserting  newline before the '{'",
                                 return_type->location);
            }
            FlagSet fn_flags = parse_flags(state);
            NodePtr body = parse_block_statement(state);
            return (new nodes::AnonymousGenerator(location, arguments, return_type, fn_flags, body))->get();
        }
    }

    NodePtr parse_block(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        NodeList nodes{};
        auto front = state.peek_skip_nl();
        while (front.ty != RightBrace && front.ty != EoF) {
            switch (front.ty) {
                case Semicolon:
                    state.eatAny();
                    break;
                default:
                    nodes.push_back(parse_block_statement(state));
                    break;
            }
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightBrace, "a '}' to close a block", error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            nodes.push_back(err.value());
        }
        return (new nodes::Block(location, nodes))->get();
    }

    NodePtr parse_structure(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        bool is_tuple = false;
        NodeList children{};
        NodeList interfaces{};
        auto front = state.peek_skip_nl();
        if (front.ty != LeftBrace && front.ty != LeftParen && front.ty != Impl) {
            return (new nodes::Structure(location, interfaces, children, is_tuple))->get();
        }
        if (front.ty == Impl) {
            state.eatAny();
            front = state.peek_skip_nl();
            while (front.ty != EoF && front.ty != LeftBrace) {
                if (front.ty == Comma) {
                    state.eatAny();
                } else {
                    auto expr = parse_expression(state);
                    if (instanceof<nodes::ObjectCall>(expr)) {
                        error::make_note("parser",
                                         "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a structure, if this is not what you want this can be fixed by inserting a comma or newline before the '{'",
                                         expr->location);
                    }
                    interfaces.push_back(expr);
                }
                front = state.peek_skip_nl();
            }
            if (front.ty != LeftBrace) {
                children.push_back(
                        state.unexpected(front, "a '{' to begin a structure", error::ErrorCode::ExpectedOpeningBrace));
                return (new nodes::Structure(location, interfaces, children, is_tuple))->get();
            }
        }
        if (front.ty == LeftParen) {
            state.eatAny();
            is_tuple = true;
            children = parse_call_list(state, RightParen);
        } else {
            state.eatAny();
            front = state.peek_skip_nl();
            bool last_was_field = false;
            while (front.ty != EoF && front.ty != RightBrace) {
                if (front.ty == Comma) {
                    if (!last_was_field) {
                        children.push_back(
                                state.raise("parser", "Expected a semicolon to separate 2 non-field declarations",
                                            front.location, error::ErrorCode::IncorrectSeparator));
                        state.eatAny();
                    } else {
                        state.eatAny();
                    }
                } else if (front.ty == Semicolon) {
                    state.eatAny();
                } else {

                    auto last = parse_statement(state);
                    children.push_back(last);
                    if (instanceof<nodes::Field>(last)) {
                        last_was_field = true;
                    } else {
                        last_was_field = false;
                    }
                }
                front = state.peek_skip_nl();
            }
            auto err = state.eat(RightBrace, "a '}' to end a structure declaration",
                                 error::ErrorCode::ExpectedClosingBrace);
            if (err.has_value()) {
                children.push_back(err.value());
            }
        }
        return (new nodes::Structure(location, interfaces, children, is_tuple))->get();
    }

    NodePtr parse_interface(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        bool is_tuple = false;
        NodeList children{};
        NodeList interfaces{};
        bool dynamic{false};
        auto front = state.peek_skip_nl();
        if (front.ty != LeftBrace && front.ty != Impl && front.ty != Dynamic) {
            return (new nodes::Interface(location, interfaces, children, dynamic))->get();
        }
        if (front.ty == Dynamic) {
            state.eatAny();
            dynamic = true;
        }
        front = state.peek_skip_nl();
        if (front.ty == Impl) {
            state.eatAny();
            front = state.peek_skip_nl();
            while (front.ty != EoF && front.ty != LeftBrace) {
                if (front.ty == Comma) {
                    state.eatAny();
                } else {
                    auto expr = parse_expression(state);
                    if (instanceof<nodes::ObjectCall>(expr)) {
                        error::make_note("parser",
                                         "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of an interface, if this is not what you want this can be fixed by inserting a comma or newline before the '{'",
                                         expr->location);
                    }
                    interfaces.push_back(expr);
                }
                front = state.peek_skip_nl();
            }
        }
        auto err = state.eat(LeftBrace, "a '{' to begin an interface", error::ErrorCode::ExpectedOpeningBrace);
        if (err.has_value()) {
            children.push_back(err.value());
        }
        front = state.peek_skip_nl();
        bool last_was_field = false;
        while (front.ty != EoF && front.ty != RightBrace) {
            if (front.ty == Comma) {
                if (!last_was_field) {
                    children.push_back(
                            state.raise("parser", "Expected a semicolon to separate 2 non-field declarations",
                                        front.location, error::ErrorCode::IncorrectSeparator));
                    state.eatAny();
                } else {
                    state.eatAny();
                }
            } else if (front.ty == Semicolon) {
                state.eatAny();
            } else {

                auto last = parse_statement(state);
                children.push_back(last);
                if (instanceof<nodes::Field>(last)) {
                    last_was_field = true;
                } else {
                    last_was_field = false;
                }
            }
            front = state.peek_skip_nl();
        }
        err = state.eat(RightBrace, "a '}' to end an interface declaration", error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            children.push_back(err.value());
        }
        return (new nodes::Interface(location, interfaces, children, dynamic))->get();
    }

    NodePtr parse_if(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto condition = parse_expression(state);
        //TODO: Figure out a way to not warn about this when the condition is parenthesized!
        if (instanceof<nodes::ObjectCall>(condition)) {
            error::make_note("parser",
                             "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of an if expression, if this is not what you want this can be fixed by inserting `then` or a newline before the '{'",
                             condition->location);
        }
        auto front = state.peek_skip_nl();
        std::optional<NodePtr> unwrapped_capture;
        if (front.ty == Colon) {
            state.eatAny();
            unwrapped_capture = parse_capture(state, false);
            if (instanceof<nodes::ObjectCall>(unwrapped_capture.value())) {
                error::make_note("parser",
                                 "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of an if expression, if this is not what you want this can be fixed by inserting `then` or a newline before the '{'",
                                 unwrapped_capture.value()->location);
            }
        }
        front = state.peek_skip_nl();
        if (front.ty == Then) {
            state.eatAny();
        }
        auto body = parse_expression(state);
        std::optional<NodePtr> els;
        front = state.peek_skip_nl();
        if (front.ty == Else) {
            state.eatAny();
            els = parse_expression(state);
        }
        return (new nodes::If(location, condition, unwrapped_capture, body, els))->get();
    }

    NodePtr parse_named_block(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        std::string name;
        NodeList nodes{};
        name = state.peek_skip_nl().value;
        auto err = state.eat(Identifier, "a name for a named block", error::ErrorCode::ExpectedName);
        if (err.has_value()) {
            if (name != ")" && name != "{") {
                state.eatAny();
            }
            nodes.push_back(err.value());
        }
        err = state.eat(RightParen, "a ')' to finish off the name section of a named block",
                        error::ErrorCode::ExpectedClosingParentheses);
        if (err.has_value()) {
            nodes.push_back(err.value());
        }
        err = state.eat(LeftBrace, "a '{' to open a named block", error::ErrorCode::ExpectedOpeningBrace);
        //Return early again in this case
        if (err.has_value()) {
            return err.value();
        }
        auto front = state.peek_skip_nl();
        while (front.ty != RightBrace && front.ty != EoF) {
            switch (front.ty) {
                case Semicolon:
                    state.eatAny();
                    break;
                default:
                    nodes.push_back(parse_block_statement(state));
                    break;
            }
            front = state.peek_skip_nl();
        }
        err = state.eat(RightBrace, "a '}' to close a block", error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            nodes.push_back(err.value());
        }
        return (new nodes::NamedBlock(location, name, nodes))->get();
    }

    NodePtr parse_destructuring_match_arm(parser_state &state);

    NodePtr parse_structure_enum_match_statement(parser_state &state, const std::string &ident, Coordinate location) {
        state.eatAny();
        NodeDict children;
        auto front = state.peek_skip_nl();
        while (front.ty != RightBrace && front.ty != EoF) {
            std::string name = static_cast<std::string>(front.value);
            front = state.peek_skip_nl();
            auto err = state.eat(Identifier, "a name for a field to match", error::ErrorCode::ExpectedName);
            if (err.has_value()) {
                state.eatAny();
                continue;
            }
            if (front.ty == Block) {
                err = state.raise("parser",
                                  "':(' is lexed as the beginning to a name block rather than ':' and '(' separately, to fix this add whitespace between ':' and '(' ",
                                  front.location, error::ErrorCode::AccidentalNamedBlock);
            } else {
                err = state.eat(Colon, "a ':' for a field match statement to begin", error::ErrorCode::ExpectedColon);
            }
            if (err.has_value()) {
                state.eatAny();
                continue;
            }
            auto arm = parse_destructuring_match_arm(state);
            children[name] = arm;
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightBrace, "a '}' to end a destructuring match statement",
                             error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            return err.value();
        } else {
            return (new nodes::MatchEnumStructure(location, children, ident))->get();
        }
    }

    NodePtr parse_tuple_enum_match_statement(parser_state &state, const std::string &ident, Coordinate location) {
        state.eatAny();
        NodeList children;
        auto front = state.peek_skip_nl();
        while (front.ty != RightParen && front.ty != EoF) {
            auto arm = parse_destructuring_match_arm(state);
            children.push_back(arm);
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightParen, "a ')' to end a destructuring match statement",
                             error::ErrorCode::ExpectedClosingParentheses);
        if (err.has_value()) {
            return err.value();
        } else {
            return (new nodes::MatchEnumTuple(location, children, ident))->get();
        }
    }

    NodePtr parse_tuple_destructuring_match_statement(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        NodeList children;
        auto front = state.peek_skip_nl();
        while (front.ty != RightParen && front.ty != EoF) {
            auto arm = parse_destructuring_match_arm(state);
            children.push_back(arm);
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightParen, "a ')' to end a destructuring match statement",
                             error::ErrorCode::ExpectedClosingParentheses);
        if (err.has_value()) {
            return err.value();
        } else {
            return (new nodes::DestructuringMatchTuple(location, children))->get();
        }
    }

    NodePtr parse_array_destructuring_match_statement(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        NodeList children;
        auto front = state.peek_skip_nl();
        while (front.ty != RightBracket && front.ty != EoF) {
            auto arm = parse_destructuring_match_arm(state);
            children.push_back(arm);
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightBracket, "a ']' to end a destructuring match statement",
                             error::ErrorCode::ExpectedClosingParentheses);
        if (err.has_value()) {
            return err.value();
        } else {
            return (new nodes::DestructuringMatchArray(location, children))->get();
        }
    }

    NodePtr parse_struct_destructuring_match_statement(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        NodeDict children;
        auto front = state.peek_skip_nl();
        while (front.ty != RightBrace && front.ty != EoF) {
            std::string name = static_cast<std::string>(front.value);
            front = state.peek_skip_nl();
            auto err = state.eat(Identifier, "a name for a field to match", error::ErrorCode::ExpectedName);
            if (err.has_value()) {
                state.eatAny();
                continue;
            }
            if (front.ty == Block) {
                err = state.raise("parser",
                                  "':(' is lexed as the beginning to a name block rather than ':' and '(' separately, to fix this add whitespace between ':' and '(' ",
                                  front.location, error::ErrorCode::AccidentalNamedBlock);
            } else {
                err = state.eat(Colon, "a ':' for a field match statement to begin", error::ErrorCode::ExpectedColon);
            }
            if (err.has_value()) {
                state.eatAny();
                continue;
            }
            auto arm = parse_destructuring_match_arm(state);
            children[name] = arm;
            front = state.peek_skip_nl();
        }
        auto err = state.eat(RightBrace, "a '}' to end a destructuring match statement",
                             error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            return err.value();
        } else {
            return (new nodes::DestructuringMatchStructure(location, children))->get();
        }
    }

    ALWAYS_INLINE NodePtr parse_destructuring_match_statement(parser_state &state) {
        auto front = state.peek_skip_nl();
        if (front.ty == Tuple) {
            return parse_tuple_destructuring_match_statement(state);
        } else if (front.ty == Array) {
            return parse_array_destructuring_match_statement(state);
        } else {
            return parse_struct_destructuring_match_statement(state);
        }
    }


    NodePtr parse_enum_destructuring_match_statement(parser_state &state, NodePtr primary) {
        auto value = std::dynamic_pointer_cast<nodes::EnumLiteral>(primary);
        auto location = primary->location;
        if (value == nullptr) {
            auto new_prim = parse_primary(state, primary);
            auto front = state.peek_skip_nl();
            if (front.ty == Dot2) {
                state.eatAny();
                auto secondary = parse_primary(state);
                return (new nodes::MatchRange(location, new_prim, secondary))->get();
            }
            return (new nodes::MatchValue(location, new_prim))->get();
        }
        auto enum_name = value->name;
        auto front = state.peek_skip_nl();
        if (front.ty == LeftBrace) {
            return parse_structure_enum_match_statement(state, enum_name, location);
        } else if (front.ty == LeftParen) {
            return parse_tuple_enum_match_statement(state, enum_name, location);
        } else {
            //This will cause a semantic error in semantic analysis.
            auto new_prim = parse_primary(state, primary);
            front = state.peek_skip_nl();
            if (front.ty == Dot2) {
                state.eatAny();
                auto secondary = parse_primary(state);
                return (new nodes::MatchRange(location, new_prim, secondary))->get();
            }
            return (new nodes::MatchValue(location, new_prim))->get();
        }
    }

    NodePtr parse_single_match_statement(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        if (front.ty == Tuple || front.ty == Object || front.ty == Array) {
            return parse_destructuring_match_statement(state);
        } else if (front.ty == Constrain) {
            state.eatAny();
            auto constraint = parse_primary(state);
            return (new nodes::MatchConstraint(location, constraint))->get();
        } else if (front.ty == Underscore) {
            state.eatAny();
            return (new nodes::MatchAll(location))->get();
        }
        auto primary = parse_primary_base(
                state); //Only parse base primaries, so no function calls and such, unless parenthesized
        front = state.peek_skip_nl();
        if (front.ty == LeftBrace || front.ty == LeftParen || front.ty == LeftBracket) {
            return parse_enum_destructuring_match_statement(state, primary);
        } else if (front.ty == Dot2) {
            state.eatAny();
            auto secondary = parse_primary(state);
            return (new nodes::MatchRange(location, primary, secondary))->get();
        }
        return (new nodes::MatchValue(location, primary))->get();
    }

    NodePtr parse_destructuring_match_arm(parser_state &state) {
        NodeList match_statements{};
        std::optional<NodePtr> capture{};
        auto front = state.peek_skip_nl();
        auto location = front.location;
        auto back_location = front.location;
        auto last_location = front.location;
        auto last_state = state.location;
        while (front.ty != Semicolon && front.ty != RightBrace && front.ty != RightBracket && front.ty != RightParen &&
               front.ty != SingleArrow && front.ty != EoF) {
            if (front.ty == Colon) {
                //In this case we know exactly what the error is, and we can easily recover from it for parsing other match statements in the future
                auto err = state.raise("parser",
                                       "missing semicolon to end destructuring match list, therefore this ':' is being parsed within the previous match arm list",
                                       front.location, error::ErrorCode::MissingSemicolon);
                error::make_note("parser",
                                 "this field identifier is being treated as another option in the previous match list",
                                 last_location);
                error::make_note("parser", "this can be fixed by adding a ';' after this match statement",
                                 back_location);
                match_statements.pop_back(); //remove the erroneous identifier
                match_statements.push_back(err);
                //Revert to previous
                state.location = last_state;
                //Return current match arm
                return (new nodes::DestructuringMatchArm(location, match_statements, capture))->get();
            } else if (front.ty == Comma) {
                state.eatAny();
                front = state.peek_skip_nl();
                continue;
            }
            back_location = last_location;
            last_state = state.location;
            last_location = front.location;
            auto last_statement = parse_single_match_statement(state);
            match_statements.push_back(last_statement);
            front = state.peek_skip_nl();
        }
        if (front.ty == SingleArrow) {
            state.eatAny();
            capture = parse_capture(state, false);
            front = state.peek_skip_nl();
        }
        if (front.ty == Semicolon) {
            state.eatAny();
        }
        return (new nodes::DestructuringMatchArm(location, match_statements, capture))->get();
    }

    NodePtr parse_match_arm(parser_state &state) {
        NodeList match_statements{};
        std::optional<NodePtr> capture{};
        NodePtr body;
        auto front = state.peek_skip_nl();
        auto location = front.location;
        while (front.ty != ThickArrow && front.ty != SingleArrow && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
                front = state.peek_skip_nl();
                continue;
            }
            match_statements.push_back(parse_single_match_statement(state));
            front = state.peek_skip_nl();
        }
        if (front.ty == SingleArrow) {
            state.eatAny();
            capture = parse_capture(state, false);
        }
        auto err = state.eat(ThickArrow, "a '=>' to begin the body of a match statement",
                             error::ErrorCode::ExpectedMatchBodySpecifier);
        if (err.has_value()) {
            body = err.value();
            //Return current match arm
            return (new nodes::MatchArm(location, match_statements, capture, body))->get();
        }
        body = parse_expression(state);
        //Return current match arm
        return (new nodes::MatchArm(location, match_statements, capture, body))->get();
    }

    NodePtr parse_match(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto value = parse_expression(state);
        NodeList arms;
        if (instanceof<nodes::ObjectCall>(value)) {
            error::make_note("parser",
                             "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a match expression, if this is not what you want this can be fixed by inserting `with` or a newline before the '{'",
                             value->location);
        }
        auto front = state.peek_skip_nl();
        if (front.ty == With) {
            state.eatAny();
        }
        auto err = state.eat(LeftBrace, "a '{' to begin a match expression", error::ErrorCode::ExpectedOpeningBrace);
        //If there is no beginning then ignore the rest of the match expression, as we shan't assume that anything was actually put here
        if (err.has_value()) {
            arms.push_back(err.value());
            return (new nodes::Match(location, value, arms))->get();
        }
        front = state.peek_skip_nl();
        while (front.ty != RightBrace && front.ty != EoF) {
            arms.push_back(parse_match_arm(state));
            front = state.peek_skip_nl();
        }
        err = state.eat(RightBrace, "a '}' to end a match expression", error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            arms.push_back(err.value());
        }
        return (new nodes::Match(location, value, arms))->get();
    }

    std::string unescape(std::string_view sv) {
        std::string result = "";
        bool last_was_escape{false};
        for (size_t i = 0; i < sv.size(); i++) {
            auto chr = sv[i];
            if (last_was_escape) {
                switch (chr) {
                    case 'a':
                        result += '\a';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'e':
                        result += '\x1b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'v':
                        result += '\v';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7': {
                        char first_octit, second_octit, third_octit;
                        first_octit = chr - '0';
                        if (i + 1 >= sv.size()) {
                            third_octit = first_octit;
                            second_octit = 0;
                            first_octit = 0;
                        } else {
                            second_octit = sv[i + 1] - '0';
                            if (7 < second_octit || second_octit < 0) {
                                third_octit = first_octit;
                                second_octit = 0;
                                first_octit = 0;
                            } else if (i + 1 >= sv.size()) {
                                i += 1;
                                third_octit = second_octit;
                                second_octit = first_octit;
                                first_octit = 0;
                            } else {
                                third_octit = sv[i + 1] - '0';
                                if (7 < third_octit || third_octit < 0) {
                                    third_octit = second_octit;
                                    second_octit = first_octit;
                                    first_octit = 0;
                                } else {
                                    i += 1;
                                }
                            }
                        }
                        char res = (first_octit << 6) | (second_octit << 3) | third_octit;
                        result += res;
                        break;
                    }
                    case 'x':
                    case 'u':
                    case 'U': {
                        int num_bytes = (chr == 'x') ? 1 : ((chr == 'u') ? 2 : 4);

                        if (i + 1 >= sv.size()) {
                            throw std::runtime_error("no hex digits following \\x");
                        }
                        std::vector<char> hex_digits;
                        while (i + 1 < sv.size() && std::isxdigit(sv[i]) && hex_digits.size() < num_bytes * 2) {
                            i += 1;
                            hex_digits.push_back(sv[i]);
                        }

                        union {
                            std::uint64_t num;
                            struct {
                                std::uint8_t bytes[8];
                            };
                        } number = {0};
                        std::uint64_t shift = (hex_digits.size() - 1) * 4;
                        for (auto digit: hex_digits) {
                            std::uint64_t v = (digit >= 'A') ? (digit >= 'a') ? (digit - 'a' + 10) : (digit - 'A' + 10)
                                                             : (digit - '0');
                            number.num |= (v << shift);
                            if (shift > 0)
                                shift -= 4;
                        }

                        for (int i = 7; i > 7 - num_bytes; i--) {
                            result += number.bytes[i];
                        }
                    }
                    default:
                        result += chr;
                        break;
                }
                last_was_escape = false;
            } else {
                if (chr == '\\') {
                    last_was_escape = true;
                } else {
                    result += chr;
                }
            }
        }
        return result;
    }

    NodePtr parse_string_literal(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        auto unescaped = unescape(front.value.substr(1, front.value.size() - 2));
        state.eatAny();
        return (new nodes::StringLiteral(location, unescaped))->get();
    }

    NodePtr parse_character_literal(parser_state &state) {
        //This language supports wide character literals so 'abcd' is treated as a 32 bit number and 'abcdefghijklmnopqrstuvwxyz' is treated as a 208 bit number
        auto front = state.peek_skip_nl();
        auto location = front.location;
        auto unescaped = unescape(front.value.substr(1, front.value.size() - 2));
        math::BigInteger number = 0;
        std::size_t shift = 0;
        for (auto chr: unescaped) {
            math::BigInteger sub{static_cast<uint64_t>(chr), shift};
            number |= sub;
            shift += 8;
        }
        state.eatAny();
        return (new nodes::IntegerLiteral(location, number))->get();
    }

    NodePtr parse_enumeration_value(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;

        if (front.ty == Underscore) {
            state.eatAny();
            return (new nodes::NonExhaustive(location))->get();
        }
        NodeList children{};
        bool is_tuple = false;
        std::string name = static_cast<std::string>(front.value);
        auto err = state.eat(Identifier, "a name for an enum member", error::ErrorCode::ExpectedName);
        if (err.has_value()) {
            state.eatAny(); // Eat this cuz otherwise infinite loop.
            return err.value();
        }
        front = state.peek_skip_nl();
        if (front.ty == LeftParen) {
            is_tuple = true;
            state.eatAny();
            children = parse_call_list(state, RightParen);
        } else if (front.ty == LeftBrace) {
            state.eatAny();
            while (front.ty != RightBrace && front.ty != EoF) {
                if (front.ty == Comma || front.ty == Semicolon) {
                    state.eatAny();
                } else {
                    children.push_back(parse_statement(state));
                }
                front = state.peek_skip_nl();
            }
            err = state.eat(RightBrace, "a '}' to close a structural enum member",
                            error::ErrorCode::ExpectedClosingBrace);
            if (err.has_value()) {
                children.push_back(err.value());
            }
        }
        front = state.peek_skip_nl();
        std::optional<NodePtr> value;
        if (front.ty == Assign) {
            state.eatAny();
            value = parse_expression(state);
        }
        return (new nodes::EnumMember(location, name, is_tuple, children, value))->get();
    }

    NodePtr parse_enumeration(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        std::optional<NodePtr> containingType;
        if (front.ty != LeftBrace) {
            containingType = parse_expression(state);
            if (instanceof<nodes::ObjectCall>(containingType.value())) {
                error::make_note("parser",
                                 "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of an enumeration, if this is not what you want this can be fixed by inserting a newline before the '{'",
                                 containingType.value()->location);
            }
        }
        auto err = state.eat(LeftBrace, "a '{' to begin an enumeration", error::ErrorCode::ExpectedOpeningBrace);
        if (err.has_value()) {
            return err.value();
        }
        front = state.peek_skip_nl();
        NodeList enumeration_values{};
        while (front.ty != RightBrace && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                enumeration_values.push_back(parse_enumeration_value(state));
            }
            front = state.peek_skip_nl();
        }
        err = state.eat(RightBrace, "a '}' to end an enumeration", error::ErrorCode::ExpectedClosingBrace);
        if (err.has_value()) {
            enumeration_values.push_back(err.value());
        }
        return (new nodes::Enum(location, containingType, enumeration_values))->get();
    }

    NodePtr parse_loop(parser_state &state);

    NodePtr parse_array_type(parser_state &state);

    NodePtr parse_slice_type(parser_state &state);

    NodePtr parse_while_loop(parser_state &state);

    NodePtr parse_for_loop(parser_state &state);

    NodePtr parse_primary_base(parser_state &state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
//        auto pi = wrap_value(parse_integer);
//#define PARSE_TO_VALUE  [location](parser_state& state, lexer::Token front) -> NodePtr
//        return state.peek_skip_nl_switch_value({
//            {DecimalLiteral, pi},
//            {OctalLiteral, pi},
//            {BinaryLiteral,pi},
//            {HexLiteral,pi},
//            {FloatingLiteral, wrap_value(parse_float)},
//            {ImaginaryLiteral,wrap_value(parse_imaginary)},
//            {UnsignedIntType, PARSE_TO_VALUE {
//                auto sub = front.value.substr(1);
//                std::uint64_t size;
//                std::stringstream ss;
//                ss << sub;
//                ss >> size;
//                state.eatAny();
//                return (new nodes::UnsignedIntType(location, size))->get();
//            }},
//            {SignedIntType,PARSE_TO_VALUE  {
//                auto sub = front.value.substr(1);
//                std::uint64_t size;
//                std::stringstream ss;
//                ss << sub;
//                ss >> size;
//                state.eatAny();
//                return (new nodes::SignedIntType(location, size))->get();
//            }},
//            {Ampersand, unary_expression<nodes::AddressOf>},
//            {Star, unary_expression<nodes::Reference>},
//            {Identifier, PARSE_TO_VALUE {
//                state.eatAny();
//                return (new nodes::ValueReference(location, static_cast<std::string>(front.value)))->get();
//            }},
//            {BuiltinReference, PARSE_TO_VALUE {
//                state.eatAny();
//                return (new nodes::BuiltinReference(location,
//                                                    static_cast<std::string>(front.value[0] == '$' ? front.value.substr(
//                                                            1) : front.value)))->get();
//            }},
//            {ConstPointer,PARSE_TO_VALUE {
//                state.eatAny();
//                auto child = parse_expression(state);
//                return (new nodes::Reference(location, child, true))->get();
//            }},
//            {Float32, single_value<nodes::Float32>},
//            {Float64, single_value<nodes::Float64>},
//            {Dash, unary_expression<nodes::UnaryMinus>},
//            {Plus,unary_expression<nodes::UnaryPlus>},
//            {Not, unary_expression<nodes::Not>},
//            {True,single_value<nodes::True>},
//            {False,single_value<nodes::False>},
//            {Void,single_value<nodes::Void>},
//            {Pipe, wrap_value(parse_closure)},
//            {Fn, wrap_value(parse_anon_function_or_type)},
//            {Generator, wrap_value(parse_anon_generator_or_type)},
//            {None,single_value<nodes::None>},
//            {Opaque,single_value<nodes::Opaque>},
//            {Complex32,single_value<nodes::Complex32>},
//            {Complex64,single_value<nodes::Complex64>},
//            {ComptimeFloat,single_value<nodes::ComptimeFloat>},
//            {ComptimeInt,single_value<nodes::ComptimeInt>},
//            {ComptimeComplex,single_value<nodes::ComptimeComplex>},
//            {ComptimeString,single_value<nodes::ComptimeString>},
//            {NoReturn,single_value<nodes::NoReturn>},
//            {Type,single_value<nodes::Type>},
//            {Tuple, PARSE_TO_VALUE {
//                state.eatAny();
//                auto args = parse_call_list(state, RightParen);
//                return (new nodes::TupleLiteral(location, args))->get();
//            }},
//            {Array, PARSE_TO_VALUE {
//                state.eatAny();
//                auto args = parse_call_list(state, RightBracket);
//                return (new nodes::ArrayLiteral(location, args))->get();
//            }},
//            {Dot, PARSE_TO_VALUE {
//                state.eatAny();
//                auto id = static_cast<std::string>(state.peek_skip_nl().value);
//                auto err = state.eat(Identifier, "an identifier for an enum literal", error::ErrorCode::ExpectedName);
//                if (err.has_value()) {
//                    return err.value();
//                }
//                return (new nodes::ComptimeEnumLiteral(location, id))->get();
//            }},
//            {Any,single_value<nodes::AnyType>},
//            {Object, PARSE_TO_VALUE {
//                state.eatAny();
//                return (new nodes::ObjectLiteral(location, parse_object_list(state)))->get();
//            }},
//            {LeftBrace,wrap_value(parse_block)},
//            {LeftParen, PARSE_TO_VALUE {
//                state.eatAny();
//                auto result = parse_expression(state);
//                static_cast<void>(state.eat(RightParen, "a ')' to close off a '('",
//                                            error::ErrorCode::ExpectedClosingParentheses));
//                return result;
//            }},
//            {DoubleThickArrow, PARSE_TO_VALUE {
//                state.eatAny();
//                front = state.peek();
//                if (front.ty == NewLine || front.ty == Semicolon || front.ty == EoF) {
//                    return (new nodes::EmptyReturn(location))->get();
//                } else if (front.ty == None) {
//                    state.eatAny();
//                    return (new nodes::EmptyReturn(location))->get();
//                } else {
//                    auto result = parse_expression(state);
//                    return (new nodes::Return(location, result))->get();
//                }
//            }},
//            {ReversedDoubleThickArrow, PARSE_TO_VALUE {
//                state.eatAny();
//                front = state.peek();
//                if (front.ty == NewLine || front.ty == Semicolon || front.ty == EoF) {
//                    return (new nodes::EmptyBreak(location))->get();
//                } else if (front.ty == None) {
//                    state.eatAny();
//                    return (new nodes::EmptyBreak(location))->get();
//                } else {
//                    auto result = parse_expression(state);
//                    return (new nodes::Break(location, result))->get();
//                }
//            }},
//            {BlockYield, PARSE_TO_VALUE {
//                state.eatAny();
//                front = state.peek();
//                auto name = static_cast<std::string>(front.value);
//                static_cast<void>(state.eat(Identifier, "a block name", error::ErrorCode::ExpectedName));
//                static_cast<void>(state.eat(RightParen, "a ')' to close off a named block yield",
//                                            error::ErrorCode::ExpectedClosingParentheses));
//                auto result = parse_expression(state);
//                return (new nodes::NamedBreak(location, name, result))->get();
//            }},
//            {Continue, single_value<nodes::Continue>},
//            {Comptime,wrap_value(parse_comptime)},
//            {Struct, wrap_value(parse_structure)},
//            {Interface,wrap_value(parse_interface)},
//            {Block, wrap_value(parse_named_block)},
//            {If,wrap_value(parse_if)},
//            {Match,wrap_value(parse_match)},
//            {Loop,wrap_value(parse_loop)},
//            {StringLiteral,wrap_value(parse_string_literal)},
//            {CharacterLiteral,wrap_value(parse_character_literal)},
//            {Enum, wrap_value(parse_enumeration)},
//            {Self,single_value<nodes::Self>},
//            {TypeSelf,single_value<nodes::SelfType>},
//            {LeftBracket,wrap_value(parse_array_type)},
//            {LessThan,wrap_value(parse_slice_type)}
//        },unexpected("any primary value",error::ErrorCode::ExpectedPrimary));
//
//#undef PARSE_TO_VALUE
        //TODO: Convert this to the functional kind
        switch (front.ty) {
            case DecimalLiteral:
            case OctalLiteral:
            case BinaryLiteral:
            case HexLiteral:
                return parse_integer(state);
            case FloatingLiteral:
                return parse_float(state);
            case ImaginaryLiteral:
                return parse_imaginary(state);
            case UnsignedIntType: {
                auto sub = front.value.substr(1);
                std::uint64_t size;
                std::stringstream ss;
                ss << sub;
                ss >> size;
                state.eatAny();
                return (new nodes::UnsignedIntType(location, size))->get();
            }
            case SignedIntType: {
                auto sub = front.value.substr(1);
                std::uint64_t size;
                std::stringstream ss;
                ss << sub;
                ss >> size;
                state.eatAny();
                return (new nodes::SignedIntType(location, size))->get();
            }
            case Ampersand: {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::AddressOf(location, child))->get();
            }
            case Star: {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::Reference(location, child, false))->get();
            }
            case Identifier: {
                state.eatAny();
                return (new nodes::ValueReference(location, static_cast<std::string>(front.value)))->get();
            }
            case BuiltinReference: {
                state.eatAny();
                return (new nodes::BuiltinReference(location,
                                                    static_cast<std::string>(front.value[0] == '$' ? front.value.substr(
                                                            1) : front.value)))->get();
            }
            case ConstPointer: {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::Reference(location, child, true))->get();
            }
            case Float32: {
                state.eatAny();
                return (new nodes::Float32(location))->get();
            }
            case Float64: {
                state.eatAny();
                return (new nodes::Float64(location))->get();
            }
            case Dash: {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::UnaryMinus(location, child))->get();
            }
            case Plus: {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::UnaryPlus(location, child))->get();
            }
            case Not: {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::Not(location, child))->get();
            }
            case Bool: {
                state.eatAny();
                return (new nodes::Bool(location))->get();
            }
            case True: {
                state.eatAny();
                return (new nodes::True(location))->get();
            }
            case False: {
                state.eatAny();
                return (new nodes::False(location))->get();
            }
            case Void: {
                state.eatAny();
                return (new nodes::Void(location))->get();
            }
            case Pipe:
                return parse_closure(state);
            case Fn:
                return parse_anon_function_or_type(state);
            case Generator:
                return parse_anon_generator_or_type(state);
            case None:
                state.eatAny();
                return (new nodes::None(location))->get();
            case Opaque:
                state.eatAny();
                return (new nodes::Opaque(location))->get();
            case Complex32:
                state.eatAny();
                return (new nodes::Complex32(location))->get();
            case Complex64:
                state.eatAny();
                return (new nodes::Complex64(location))->get();
            case ComptimeFloat:
                state.eatAny();
                return (new nodes::ComptimeFloat(location))->get();
            case ComptimeComplex:
                state.eatAny();
                return (new nodes::ComptimeComplex(location))->get();
            case ComptimeInt:
                state.eatAny();
                return (new nodes::ComptimeInt(location))->get();
            case ComptimeString:
                state.eatAny();
                return (new nodes::ComptimeString(location))->get();
            case NoReturn:
                state.eatAny();
                return (new nodes::NoReturn(location))->get();
            case Type:
                state.eatAny();
                return (new nodes::Type(location))->get();
            case Tuple: {
                state.eatAny();
                auto args = parse_call_list(state, RightParen);
                return (new nodes::TupleLiteral(location, args))->get();
            }
            case Array: {
                state.eatAny();
                auto args = parse_call_list(state, RightBracket);
                return (new nodes::ArrayLiteral(location, args))->get();
            }
            case Dot: {
                state.eatAny();
                auto id = static_cast<std::string>(state.peek_skip_nl().value);
                auto err = state.eat(Identifier, "an identifier for an enum literal", error::ErrorCode::ExpectedName);
                if (err.has_value()) {
                    return err.value();
                }
                return (new nodes::EnumLiteral(location, id))->get();
            }
            case Any: {
                state.eatAny();
                return (new nodes::AnyType(location))->get();
            }
            case Object: {
                state.eatAny();
                return (new nodes::ObjectLiteral(location, parse_object_list(state)))->get();
            }
            case LeftBrace: {
                return parse_block(state);
            }
            case LeftParen: {
                state.eatAny();
                auto result = parse_expression(state);
                static_cast<void>(state.eat(RightParen, "a ')' to close off a '('",
                                            error::ErrorCode::ExpectedClosingParentheses));
                return result;
            }
            case DoubleThickArrow: {
                state.eatAny();
                front = state.peek();
                if (front.ty == NewLine || front.ty == Semicolon || front.ty == EoF) {
                    return (new nodes::EmptyReturn(location))->get();
                } else if (front.ty == None) {
                    state.eatAny();
                    return (new nodes::EmptyReturn(location))->get();
                } else {
                    auto result = parse_expression(state);
                    return (new nodes::Return(location, result))->get();
                }
            }
            case ReversedDoubleThickArrow: {
                state.eatAny();
                front = state.peek();
                if (front.ty == NewLine || front.ty == Semicolon || front.ty == EoF) {
                    return (new nodes::EmptyBreak(location))->get();
                } else if (front.ty == None) {
                    state.eatAny();
                    return (new nodes::EmptyBreak(location))->get();
                } else {
                    auto result = parse_expression(state);
                    return (new nodes::Break(location, result))->get();
                }
            }
            case BlockYield: {
                state.eatAny();
                front = state.peek();
                auto name = static_cast<std::string>(front.value);
                static_cast<void>(state.eat(Identifier, "a block name", error::ErrorCode::ExpectedName));
                static_cast<void>(state.eat(RightParen, "a ')' to close off a named block yield",
                                            error::ErrorCode::ExpectedClosingParentheses));
                auto result = parse_expression(state);
                return (new nodes::NamedBreak(location, name, result))->get();
            }
            case Continue:
                state.eatAny();
                return (new nodes::Continue(location))->get();
            case Break:
                state.eatAny();
                return (new nodes::EmptyBreak(location))->get();
            case Comptime:
                return parse_comptime(state);
            case Struct:
                return parse_structure(state);
            case Interface:
                return parse_interface(state);
            case Block:
                return parse_named_block(state);
            case If:
                return parse_if(state);
            case Match:
                return parse_match(state);
            case Loop:
                return parse_loop(state);
            case StringLiteral:
                return parse_string_literal(state);
            case CharacterLiteral:
                return parse_character_literal(state);
            case Enum:
                return parse_enumeration(state);
            case Self:
                state.eatAny();
                return (new nodes::Self(location))->get();
            case TypeSelf:
                state.eatAny();
                return (new nodes::SelfType(location))->get();
            case LeftBracket:
                return parse_array_type(state);
            case LessThan:
                return parse_slice_type(state);
            case While:
                return parse_while_loop(state);
            case For:
                return parse_for_loop(state);
            case Underscore:
                state.eatAny();
                return (new nodes::Underscore(location))->get();
            default:
                auto res = state.unexpected(front, "any primary value", error::ErrorCode::ExpectedPrimary);
                if (front.ty == EoF) {
                    return res;
                }
                state.eatAny();
                return parse_primary_base(state);
        }
    }

    NodePtr parse_for_loop(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto capture = parse_capture(state,false);
        std::optional<NodePtr> index;
        auto front = state.peek_skip_nl();
        if (front.ty == Comma) {
            state.eatAny();
        }
        front = state.peek_skip_nl();
        if (front.ty == Identifier || front.ty == Underscore) {
            index = (new nodes::ValueReference(front.location,static_cast<std::string>(front.value)))->get();
            state.eatAny();
        } else if (front.ty != Colon) {
            index = state.unexpected(front, "an identifier or '_'",error::ErrorCode::ExpectedName);
            if (front.ty == EoF) {
                return index.value();
            }
            if (front.ty != Colon)
                state.eatAny();
        }
        auto val = state.eat(Colon,"a ':'",error::ErrorCode::ExpectedColon);
        if (val.has_value()) {
            return val.value();
        }
        auto iterable = parse_expression(state);
        NodeList transformations;
        front = state.peek_skip_nl();
        while (front.ty == Colon || front.ty == Question) {
            bool is_map = front.ty == Colon;
            state.eatAny();
            auto transform = parse_expression(state);
            if (is_map) {
                transformations.push_back((new nodes::MapTransformation(front.location,transform))->get());
            } else {
                transformations.push_back((new nodes::FilterTransformation(front.location,transform))->get());
            }
            front = state.peek_skip_nl();
        }
        if (front.ty == Do) {
            state.eatAny();
        }
        auto body = parse_expression(state);
        front = state.peek_skip_nl();
        std::optional<NodePtr> els;
        if (front.ty == Else) {
            state.eatAny();
            els = parse_expression(state);
        }
        return (new nodes::For(location,capture,index,iterable,transformations,body,els))->get();
    }

    NodePtr parse_while_loop(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto condition = parse_expression(state);
        auto front = state.peek_skip_nl();
        if (front.ty == Do) {
            state.eatAny();
        }
        auto body = parse_expression(state);
        front = state.peek_skip_nl();
        if (front.ty == Else) {
            state.eatAny();
            return (new nodes::While(location,condition,body, parse_expression(state)))->get();
        } else {
            return (new nodes::While(location,condition,body,std::optional<NodePtr>{}))->get();
        }
    }

    NodePtr parse_array_argument(parser_state &state) {
        auto front = state.peek_skip_nl();
        if (front.ty == Underscore) {
            state.eatAny();
            return (new nodes::InferredSize(front.location))->get();
        } else if (front.ty == Question) {
            state.eatAny();
            return (new nodes::UnknownSize(front.location))->get();
        } else {
            return parse_expression(state);
        }
    }

    NodePtr parse_slice_type(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        if (front.ty == GreaterThan) {
            state.eatAny();
            return (new nodes::Slice(location, parse_primary(state),false))->get();
        } else if (front.ty == ConstSlice) {
            state.eatAny();
            return (new nodes::Slice(location, parse_primary(state),true))->get();
        } else {
            return state.unexpected(front,"> or >~",error::ErrorCode::ExpectedSliceClose);
        }
    }

    NodePtr parse_array_type(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        NodeList array_args{};
        auto front = state.peek_skip_nl();
        while (front.ty != RightBracket && front.ty != ConstArray && front.ty != EoF) {
            if (front.ty != Comma) {
                array_args.push_back(parse_array_argument(state)); // Parse the argument
            } else {
                state.eatAny();
            }
            front = state.peek_skip_nl();
        }
        if (front.ty == EoF) {
            return state.unexpected(front,"a ] or ]~ to close an array",error::ErrorCode::ExpectedClosingBracket);
        }
        bool constant = front.ty == ConstArray;
        state.eatAny();
        auto type = parse_primary(state);
        if (array_args.size() == 0) {
            return (new nodes::ImplicitArray(location,type,constant))->get();
        } else {
            return (new nodes::ArrayType(location,array_args,type,constant))->get();
        }
    }


    NodePtr parse_loop(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        return (new nodes::Loop(location, parse_block_statement(state)))->get();
    }

    bool valid_assignment_operator(lexer::TokenType ty) {
        switch (ty) {
            case Redefine:
            case Assign:
            case AddAssign:
            case SubAssign:
            case MulAssign:
            case DivAssign:
            case ModAssign:
            case XorAssign:
            case OrAssign:
            case AndAssign:
            case LeftShiftAssign:
            case RightShiftAssign:
                return true;
            default:
                return false;
        }
    }

    NodePtr parse_block_statement(parser_state &state) {
        auto front = state.peek_skip_nl();
        switch (front.ty) {
            case Let:
                return parse_variable_decl(state);
            case Def:
                return parse_definition(state);
            default: {
                auto base = parse_expression(state);
                front = state.peek();
                if (valid_assignment_operator(front.ty)) {
                    state.eatAny();
                    auto value = parse_expression(state);
                    return create_node_from_binary_operator(front, base, value);
                }
                return base;
            }
        }
    }

    std::string get_operator_type(parser_state &state) {
        auto front = state.peek_skip_nl();
        switch (front.ty) {
        case LeftParen:
            state.eatAny();
            static_cast<void>(state.eat(RightParen,"A ')'",error::ErrorCode::ExpectedClosingParentheses));
            return "tuple_call";
        case LeftBracket:
            state.eatAny();
            static_cast<void>(state.eat(RightBracket,"A ']'",error::ErrorCode::ExpectedClosingBracket));
            return "array_call";
        case LeftBrace:
            state.eatAny();
            static_cast<void>(state.eat(RightBrace,"A '}'",error::ErrorCode::ExpectedClosingBrace));
            return "object_call";
        case Dot:
            state.eatAny();
            return "subscript";
        case Plus:
            state.eatAny();
            {
                front = state.peek_skip_nl();
                if (front.ty == Question) {
                    state.eatAny();
                    return "unary_plus";
                } else {
                    return "add";
                }
            }
        case Dash:
            state.eatAny();
            {
                front = state.peek_skip_nl();
                if (front.ty == Question) {
                    state.eatAny();
                    return "unary_minus";
                } else {
                    return "subtract";
                }
            }
        case Dereference:
            state.eatAny();
            return "dereference";
        case Not:
            state.eatAny();
            return "not";
        case Star:
            state.eatAny();
            return "multiply";
        case Slash:
            state.eatAny();
            return "divide";
        case Percent:
            state.eatAny();
            return "modulate";
        case LeftShift:
            state.eatAny();
            return "shift_left";
        case RightShift:
            state.eatAny();
            return "shift_right";
        case LessThan:
            state.eatAny();
            return "lesser";
        case GreaterThan:
            state.eatAny();
            return "greater";
        case LessThanEqual:
            state.eatAny();
            return "lesser_equal";
        case GreaterThanEqual:
            state.eatAny();
            return "greater_equal";
        case EqualTo:
            state.eatAny();
            return "equal";
        case NotEqualTo:
            state.eatAny();
            return "not_equal";
        case And:
            state.eatAny();
            return "and";
        case Or:
            state.eatAny();
            return "or";
        case Xor:
            state.eatAny();
            return "xor";
        case Assign:
            state.eatAny();
            return "assign";
        case AddAssign:
            state.eatAny();
            return "add_assign";
        case SubAssign:
            state.eatAny();
            return "subtract_assign";
        case MulAssign:
            state.eatAny();
            return "multiply_assign";
        case DivAssign:
            state.eatAny();
            return "divide_assign";
        case ModAssign:
            state.eatAny();
            return "modulate_assign";
        case LeftShiftAssign:
            state.eatAny();
            return "shift_left_assign";
        case RightShiftAssign:
            state.eatAny();
            return "shift_right_assign";
        case AndAssign:
            state.eatAny();
            return "and_assign";
        case OrAssign:
            state.eatAny();
            return "or_assign";
        case XorAssign:
            state.eatAny();
            return "xor_assign";
        default:
            return "unknown_operator";
        }
    }

    NodePtr parse_operator(parser_state &state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        //At the top level we don't expect functional types or the such here
        auto op = get_operator_type(state);
        if (op == "unknown_operator") {
            static_cast<void>(state.raise("parser","Unknown operator to overload",location,error::ErrorCode::UnknownOperator));
        }
        NodeList arguments{};
        auto front = state.peek_skip_nl();
        while (front.ty != ThickArrow && front.ty != EoF) {
            if (front.ty == Comma) {
                state.eatAny();
            } else {
                auto argument = parse_argument(state);
                if (instanceof<nodes::ErrorNode>(argument)) {
                    //Continue until the next argument begins
                    front = state.peek_skip_nl();
                    while (front.ty != ThickArrow && front.ty != Identifier && front.ty != Underscore &&
                           front.ty != EoF) {
                        state.eatAny();
                        front = state.peek_skip_nl();
                    }
                }
                arguments.push_back(argument);
            }
            front = state.peek_skip_nl();
        }
        static_cast<void>(state.eat(ThickArrow, "a '=>' to denote the return type",
                                    error::ErrorCode::ExpectedReturnSpecifier));
        NodePtr return_type = parse_expression(state);
        if (instanceof<nodes::ObjectCall>(return_type)) {
            error::make_note("parser",
                             "possibly ambiguous syntax, '{' is being treated as an object call rather than the body of a function, if this is not what you want this can be fixed by inserting a newline before the '{'",
                             return_type->location);
        }
        FlagSet fn_flags = parse_flags(state);
        auto body = parse_block_statement(state);
        return (new nodes::Operator(location, op, arguments, return_type, fn_flags,
                                    body))->get();
    }
}