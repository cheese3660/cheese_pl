//
// Created by Lexi Allen on 9/21/2022.
//
#include "parser/parser.h"
#include "error.h"
#include "parser/nodes/Structure.h"
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
        NodeList children{};
        auto& front = state.peek_skip_nl();
        auto location = front.location;
        return (new nodes::Structure(location, children,false))->get();
    }

    NodePtr parse_import(parser_state& state);
    NodePtr parse_function(parser_state& state);
    NodePtr parse_variable_decl(parser_state& state);
    NodePtr parse_field(parser_state& state);
    NodePtr parse_definition(parser_state& state);
    NodePtr parse_comptime(parser_state& state);
    NodePtr parse_statement(parser_state& state) {
        auto& front = state.peek_skip_nl();
        switch (front.ty) {
            case Import:
                return parse_import(state);
            case Fn:
                return parse_function(state);
            case Let:
                return parse_variable_decl(state);
            case Identifier:
                return parse_field(state);
            case Def:
                return parse_definition(state);
            case Comptime:
                return parse_comptime(state);
            default:
                unexpected(front, "a structure statement (i.e. an import, function declaration, variable declaration, variable definition, comptime, or a field)", error::ErrorCode::ExpectedStructureStatement);
        }
    }

    NodePtr parse_import(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_function(parser_state& state) {
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

}