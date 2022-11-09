//
// Created by Lexi Allen on 9/21/2022.
//
#include "parser/parser.h"
#include "error.h"
#include "parser/nodes/Structure.h"
#include "parser/nodes/Import.h"
#include "NotImplementedException.h"
namespace cheese::parser {
    using enum lexer::TokenType;
    [[noreturn]] void unexpected(lexer::Token& token, std::string expected, cheese::error::ErrorCode code) {
        if (token.ty == EoF) {
            error::raise_exiting_error("parser","unexpected EOF. expected: " + expected, token.location, code);
        } else if (token.ty == StringLiteral || token.ty == CharacterLiteral) {
            error::raise_exiting_error("parser", "unexpected token " + std::string(token.value) + ", expected: " + expected,token.location, code);
        } else if (token.ty == NewLine) {
            error::raise_exiting_error("parser","unexpected newline. expected: " + expected, token.location, code);
        } else {
            error::raise_exiting_error("parser", "unexpected token '" + std::string(token.value) + "', expected: " + expected,token.location, code);
        }
    }

    struct parser_state {
        std::vector<lexer::Token>& tokens;
        size_t location=0;
        [[nodiscard]] lexer::Token& peek();
        [[nodiscard]] lexer::Token& peek_skip_nl();
        void eat(lexer::TokenType expected_type, std::string expected_message, cheese::error::ErrorCode code);
        void eatAny();
    };

    lexer::Token& parser_state::peek() {
        return tokens[0];
    }

    lexer::Token& parser_state::peek_skip_nl() {
        while (peek().ty == NewLine) {
            static_cast<void>(eatAny());
        }
        return peek();
    }

    void parser_state::eatAny() {
        if (location < tokens.size() - 1) location++;
    }

    void parser_state::eat(lexer::TokenType expected_type, std::string expected_message, cheese::error::ErrorCode code) {
        auto& token = peek();
        eatAny();
        if (token.ty != expected_type) {
            unexpected(token,expected_message,code);
        }
    }

    NodePtr parse_program(parser_state& state);
    NodePtr parse(std::vector<lexer::Token> &tokens) {
        parser_state state{tokens,0};
        return parse_program(state);
    }

    NodePtr parse_statement(parser_state& state);
    NodePtr parse_program(parser_state& state) {
        NodeList interfaces{};
        NodeList children{};
        auto& front = state.peek_skip_nl();
        auto location = front.location;
        return (new nodes::Structure(location, interfaces, children,false))->get();
    }

    NodePtr parse_import(parser_state& state);
    NodePtr parse_function(parser_state& state);
    NodePtr parse_generator(parser_state& state);
    NodePtr parse_variable_decl(parser_state& state);
    NodePtr parse_field(parser_state& state);
    NodePtr parse_definition(parser_state& state);
    NodePtr parse_comptime(parser_state& state);
    NodePtr parse_expression(parser_state& state);
    NodePtr parse_statement(parser_state& state) {
        start:
        auto& front = state.peek_skip_nl();
        switch (front.ty) {
            case Import:
                return parse_import(state);
            case Fn:
                return parse_function(state);
            case Generator:
                return parse_generator(state);
            case Let:
                return parse_variable_decl(state);
            case Identifier:
                return parse_field(state);
            case Def:
                return parse_definition(state);
            case Comptime:
                return parse_comptime(state);
//            case BuiltinReference:
//                return parse_expression(state);
            case Comma:
            case Semicolon:
                state.eatAny();
                goto start;
            default:
                unexpected(front, "a structure statement (i.e. an import, function declaration, variable declaration, variable definition, comptime, or a field)", error::ErrorCode::ExpectedStructureStatement);
        }
    }

    NodePtr parse_import(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        //Get the actual import path
        std::string path = "";
        std::string resolved_name = "";
        std::string last_name = "";
        while (true) {
            auto front = state.peek_skip_nl();
            if (front.ty != Dot && front.ty != Dereference && front.ty != Identifier) {
                break;
            }
            else if (front.ty == Dot) {
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
            error::raise_exiting_error("parser","Missing import path following import statement", location, error::ErrorCode::ExpectedImportPath);
        }
        auto front = state.peek_skip_nl();
        if (front.ty == Cast) {
            state.eatAny();
            front = state.peek_skip_nl();
            resolved_name = front.value;
            state.eat(Identifier,"an identifier following an '@' for an import statement", error::ErrorCode::ExpectedImportName);
        } else {
            resolved_name = last_name;
        }
        return (new nodes::Import(location,path,resolved_name))->get();
    }
    NodePtr parse_argument(parser_state& state) {

    }
    NodePtr parse_function(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        NodeList arguments{};


        //At the top level we don't expect functional types or the such here
        auto name = state.peek_skip_nl().value;
        state.eat(Identifier,"a function name for a structure level 'fn'",error::ErrorCode::ExpectedFunctionGeneratorName);
        auto front = state.peek_skip_nl();

        while (front.ty != ThickArrow) {
            if (front.ty == Comma) continue;
            else {
                arguments.push_back(parse_argument(state));
            }
            front = state.peek_skip_nl();
        }
        state.eatAny();
        NodePtr return_type = parse_expression(parser_state& state);
    }

    NodePtr parse_generator(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_variable_decl(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_field(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_definition(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_comptime(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_expression(parser_state& state) {
        NOT_IMPL
    }

}