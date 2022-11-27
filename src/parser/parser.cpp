//
// Created by Lexi Allen on 9/21/2022.
//
#include "parser/parser.h"
#include "error.h"
#include "parser/nodes/TerminalNodes.h"
#include "parser/nodes/SingleMemberNodes.h"
#include "parser/nodes/OtherNodes.h"
#include "NotImplementedException.h"
#include <sstream>
template<typename Base, typename T>
static inline bool instanceof(const T ptr) {
    return std::dynamic_pointer_cast<Base>(ptr).get() != nullptr;
}
namespace cheese::parser {


    using enum lexer::TokenType;
    [[noreturn]] void unexpected(lexer::Token& token, const std::string& expected, cheese::error::ErrorCode code) {
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
        void eat(lexer::TokenType expected_type, const std::string& expected_message, cheese::error::ErrorCode code);
        void eatAny();
    };

    lexer::Token& parser_state::peek() {
        return tokens[location];
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

    void parser_state::eat(lexer::TokenType expected_type, const std::string& expected_message, cheese::error::ErrorCode code) {
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
        bool last_was_field = false;
        while (front.ty != EoF) {
            if (front.ty == Comma) {
                if (!last_was_field) {
                    error::raise_exiting_error("parser","Expected a semicolon to separate 2 non-field declarations", front.location,error::ErrorCode::IncorrectSeparator);
                } else {
                    state.eatAny();
                }
            } else if (front.ty == Semicolon) {
                state.eatAny();
            }
            auto last = parse_statement(state);
            children.push_back(last);
            if (instanceof<nodes::Field>(last)) {
                last_was_field = true;
            } else {
                last_was_field = false;
            }
            front = state.peek_skip_nl();
        }
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
    NodePtr parse_unnamed_field(parser_state& state);
    NodePtr parse_block_statement(parser_state& state);
    NodePtr parse_statement(parser_state& state) {
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
            case Underscore:
                return parse_unnamed_field(state);
            case Def:
                return parse_definition(state);
            case Comptime:
                return parse_comptime(state);
//            case BuiltinReference:
//                return parse_expression(state);
            default:
                unexpected(front, "a structure statement (i.e. an import, function declaration, variable declaration, variable definition, comptime, or a field)", error::ErrorCode::ExpectedStructureStatement);
        }
    }

    NodePtr parse_import(parser_state& state) {
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
        auto front = state.peek_skip_nl();
        auto location = front.location;
        if (front.ty == Semicolon) {
            error::raise_exiting_error("parser","expected ',' as a separator, not ';'",front.location,error::ErrorCode::IncorrectSeparator);
        }
        if (front.ty != Underscore && front.ty != Identifier) {
            unexpected(front,"'_' or IDENT",error::ErrorCode::ExpectedName);
        }
        std::optional<std::string> name;
        if (front.ty == Identifier) {
            name = static_cast<std::string>(front.value);
        }
        state.eatAny();
        state.eat(Colon,"a ':' to denote the argument type", error::ErrorCode::ExpectedColon);
        auto type = parse_expression(state);
        bool comptime = state.peek_skip_nl().ty == Comptime;
        if (comptime) {
            state.eatAny();
        }
        return (new nodes::Argument(location,name,type,comptime))->get();
    }

    bool valid_flag(lexer::TokenType t) {
        return t == Inline || t == Extern || t == Export || t == Comptime || t == Public || t == Private || t == Mutable || t == Entry;
    }

    FlagSet parse_flags(parser_state& state) {
        FlagSet f{false,false,false,false,false,false,false,false};
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
                    //unreachable
                    break;
            }
            state.eatAny();
            front = state.peek_skip_nl();
        }


        return f;
    }

    NodePtr parse_function(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        //At the top level we don't expect functional types or the such here
        auto name = state.peek_skip_nl().value;
        state.eat(Identifier,"a function name for a structure level 'fn'",error::ErrorCode::ExpectedFunctionGeneratorName);
        NodeList arguments{};
        auto front = state.peek_skip_nl();
        while (front.ty != ThickArrow) {
            if (front.ty == Comma) {
                state.eatAny();
            }
            else {
                arguments.push_back(parse_argument(state));
            }
            front = state.peek_skip_nl();
        }
        state.eatAny();
        NodePtr return_type = parse_expression(state);
        FlagSet fn_flags = parse_flags(state);
        if (state.peek_skip_nl().ty == Prototype) {
            state.eatAny();
            return (new nodes::FunctionPrototype(location,static_cast<std::string>(name),arguments,return_type,fn_flags))->get();
        } else if (state.peek_skip_nl().ty == Import) {
            state.eatAny();
            return (new nodes::FunctionImport(location,static_cast<std::string>(name),arguments,return_type,fn_flags))->get();
        } else {
            auto body = parse_block_statement(state);
            return (new nodes::Function(location,static_cast<std::string>(name),arguments,return_type,fn_flags,body))->get();
        }
    }

    NodePtr parse_generator(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        //At the top level we don't expect functional types or the such here
        auto name = state.peek_skip_nl().value;
        state.eat(Identifier,"a generator name for a structure level 'generator'",error::ErrorCode::ExpectedFunctionGeneratorName);
        NodeList arguments{};
        auto front = state.peek_skip_nl();
        while (front.ty != ThickArrow) {
            if (front.ty == Comma) {
                state.eatAny();
            }
            else {
                arguments.push_back(parse_argument(state));
            }
            front = state.peek_skip_nl();
        }
        state.eatAny();
        NodePtr return_type = parse_expression(state);
        FlagSet fn_flags = parse_flags(state);
        if (state.peek_skip_nl().ty == Prototype) {
            state.eatAny();
            return (new nodes::GeneratorPrototype(location,static_cast<std::string>(name),arguments,return_type,fn_flags))->get();
        } else if (state.peek_skip_nl().ty == Import) {
            state.eatAny();
            return (new nodes::GeneratorImport(location,static_cast<std::string>(name),arguments,return_type,fn_flags))->get();
        } else {
            auto body = parse_block_statement(state);
            return (new nodes::Generator(location,static_cast<std::string>(name),arguments,return_type,fn_flags,body))->get();
        }
    }

    NodePtr parse_def(parser_state& state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        auto name = front.value;
        state.eat(Identifier,"a variable name",error::ErrorCode::ExpectedName);
        front = state.peek_skip_nl();
        std::optional<NodePtr> ty;
        if (front.ty == Colon) {
            state.eatAny();
            ty = parse_expression(state);
        }
        auto flags = parse_flags(state);

        return (new nodes::VariableDefinition(location,static_cast<std::string>(name),ty,flags))->get();
    }

    NodePtr parse_definition(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto def = parse_def(state);
        def->location = location;
        return def;
    }

    NodePtr parse_destructure_statement(parser_state& state);

    NodePtr parse_destructure_array(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeList children{};
        while (front.ty != RightBracket) {
            if (front.ty == Comma) {
                state.eatAny();
            }
            else {
                children.push_back(parse_destructure_statement(state));
            }
            front = state.peek_skip_nl();
        }
        state.eatAny();
        return (new nodes::ArrayDestructure(location,children))->get();
    }

    NodePtr parse_destructure_slice(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeList children{};
        while (front.ty != GreaterThan) {
            if (front.ty == Comma) {
                state.eatAny();
            }
            else {
                children.push_back(parse_destructure_statement(state));
            }
            front = state.peek_skip_nl();
        }
        state.eatAny();
        return (new nodes::SliceDestructure(location,children))->get();
    }

    NodePtr parse_destructure_tuple(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeList children{};
        while (front.ty != RightParen) {
            if (front.ty == Comma) {
                state.eatAny();
            }
            else {
                children.push_back(parse_destructure_statement(state));
            }
            front = state.peek_skip_nl();
        }
        state.eatAny();
        return (new nodes::TupleDestructure(location,children))->get();
    }

    NodePtr parse_destructure_structure(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto front = state.peek_skip_nl();
        NodeDict children;
        while (front.ty != RightBrace) {
            if (front.ty == Comma) {
                state.eatAny();
            } else if (front.ty == Underscore || front.ty == Identifier) {
                auto name = static_cast<std::string>(front.value);
                state.eatAny();
                state.eat(Colon,"a ':'",error::ErrorCode::ExpectedColon);
                auto value = parse_destructure_statement(state);
                children[name] = value;
            } else {
                unexpected(front,"a structure destructuring specifier",error::ErrorCode::ExpectedDestructuringStatement);
            }
            front = state.peek_skip_nl();
        }
        state.eatAny();
        return (new nodes::StructureDestructure(location,children))->get();
    }

    NodePtr parse_destructure_statement(parser_state& state) {
        auto front = state.peek_skip_nl();
        switch (front.ty) {
            case Underscore:
                state.eatAny();
                return (new nodes::Underscore(front.location))->get();
            case LeftBracket:
                return parse_destructure_array(state);
            case LessThan:
                return parse_destructure_slice(state);
            case LeftParen:
                return parse_destructure_tuple(state);
            case LeftBrace:
                return parse_destructure_structure(state);
            default:
                return parse_def(state);
        }
    }

    NodePtr parse_destructure(parser_state& state, Coordinate location) {
        auto structure = parse_destructure_statement(state);
        state.eat(Assign,"an '=' for a variable destructuring",error::ErrorCode::UninitializedDeclaration);
        auto value = parse_expression(state);
        return (new nodes::Destructure(location,structure,value))->get();
    }

    NodePtr parse_variable_decl(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        auto fty = state.peek_skip_nl().ty;
        if (fty == LeftBrace || fty == LeftBracket || fty == LeftParen || fty == Underscore || fty == LessThan) {
            return parse_destructure(state,location);
        }

        auto def = parse_def(state);
        state.eat(Assign,"an '=' for a variable declaration",error::ErrorCode::UninitializedDeclaration);
        auto val = parse_expression(state);
        return (new nodes::VariableDeclaration(location,def,val))->get();
    }

    NodePtr parse_field(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        std::optional<std::string> name = static_cast<std::string>(state.peek_skip_nl().value);
        state.eatAny();
        state.eat(Colon,"a ':' for a field declaration",error::ErrorCode::ExpectedColon);
        auto type = parse_expression(state);
        auto flags = parse_flags(state);

        return (new nodes::Field(location,name,type,flags))->get();
    }

    NodePtr parse_unnamed_field(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        std::optional<std::string> name;
        state.eatAny();
        state.eat(Colon,"a ':' for a field declaration",error::ErrorCode::ExpectedColon);
        auto type = parse_expression(state);
        auto flags = parse_flags(state);

        return (new nodes::Field(location,name,type,flags))->get();
    }


    NodePtr parse_comptime(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        return (new nodes::Comptime(location, parse_expression(state)))->get();
    }
    NodePtr parse_primary(parser_state& state);
    NodePtr parse_expression(parser_state& state, NodePtr old_lhs, uint8_t min_precedence);

    bool is_binary_operation(lexer::TokenType ty) {
        switch (ty) {
            case Dot: //Subscription
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
            case Dot:
                return 11;
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

    NodePtr create_node_from_binary_operator(lexer::Token tok, const NodePtr& lhs, const NodePtr& rhs) {
        switch (tok.ty) {
            case Dot:
                RETURN_NODE(Subscription)
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
                RETURN_NODE(Assingment)
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

    NodePtr parse_expression(parser_state& state) {
        return parse_expression(state,parse_primary(state),0);
    }

    NodePtr parse_expression(parser_state& state, NodePtr old_lhs, uint8_t min_precedence) {
        auto lookahead = state.peek();
        auto lhs = std::move(old_lhs);
        while (is_binary_operation(lookahead.ty) && precedence(lookahead.ty) >= min_precedence) {
            auto op = lookahead;
            auto op_prec = precedence(op.ty);
            state.eatAny();
            auto rhs = parse_primary(state);
            lookahead = state.peek();
            while (is_binary_operation(lookahead.ty) && precedence(lookahead.ty) > op_prec) {
                rhs = parse_expression(state,rhs,min_precedence+1);
                lookahead = state.peek();
            }
            lhs = create_node_from_binary_operator(op,lhs,rhs);
        }
        return lhs;
    }

    NodePtr parse_primary_base(parser_state& state);

    NodeList parse_call_list(parser_state& state, lexer::TokenType ending_type) {
        NodeList list;
        while (state.peek_skip_nl().ty != ending_type) {
            if (state.peek_skip_nl().ty == Comma) {
                state.eatAny();
                continue;
            }
            list.push_back(parse_expression(state));
        }
        state.eatAny();
        return list;
    }

    NodeDict parse_object_list(parser_state& state) {
        NodeDict dict;
        while (state.peek_skip_nl().ty != RightBrace) {
            if (state.peek_skip_nl().ty == Comma) {
                state.eatAny();
                continue;
            }
            std::string name = static_cast<std::string>(state.peek_skip_nl().value);
            state.eat(Identifier,"a field name for an object",error::ErrorCode::ExpectedName);
            state.eat(Colon,"a ':' for an object",error::ErrorCode::ExpectedColon);
            NodePtr value = parse_expression(state);
            dict[name] = value;
        }
        state.eatAny();
        return dict;
    }

    NodePtr parse_primary(parser_state& state) {
        auto base = parse_primary_base(state);
        while (true) {
            auto front = state.peek();
            auto should_break = false;
            switch (front.ty) {
                case LeftParen:
                {
                    auto location = front.location;
                    state.eatAny();
                    auto list = parse_call_list(state,RightParen);
                    base = (new nodes::TupleCall(location,base,list))->get();
                    break;
                }
                case LeftBracket:
                {
                    auto location = front.location;
                    state.eatAny();
                    auto list = parse_call_list(state,RightBracket);
                    base = (new nodes::ArrayCall(location,base,list))->get();
                    break;
                }
                case LeftBrace:
                {
                    auto location = front.location;
                    state.eatAny();
                    auto nodes = parse_object_list(state);
                    base = (new nodes::ObjectCall(location,base,nodes))->get();
                    break;
                }
                case Dereference:
                {
                    auto location = front.location;
                    state.eatAny();
                    base = (new nodes::Dereference(location,base))->get();
                    break;
                }
                default:
                    should_break = true;
            }
            if (should_break) break;
        }
        return base;
    }

    NodePtr parse_integer(parser_state& state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        state.eatAny();
        return (new nodes::IntegerLiteral(location,math::BigInteger(front.value)))->get();
    }

    NodePtr parse_float(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_imaginary(parser_state& state) {
        NOT_IMPL
    }

    NodePtr parse_capture(parser_state& state) {
        int capture_type; //0 == '=', 1 == '*', 2 == '*~'
        auto front = state.peek_skip_nl();
        auto location = front.location;
        if (front.ty == Assign) {
            capture_type = 0;
        } else if (front.ty == Star) {
            capture_type = 1;
        } else if (front.ty == ConstPointer) {
            capture_type = 2;
        } else {
            unexpected(front,"'*', '*~', '=' for a capture",error::ErrorCode::ExpectedCaptureSpecifier);
        }
        state.eatAny();
        front = state.peek_skip_nl();
        if (front.ty == Identifier) {
            auto name = static_cast<std::string>(front.value);
            state.eatAny();
            if (capture_type == 0) {
                return (new nodes::CopyCapture(location,name))->get();
            } else if (capture_type == 1) {
                return (new nodes::RefCapture(location,name))->get();
            } else {
                return (new nodes::ConstRefCapture(location,name))->get();
            }
        } else {
            if (capture_type == 0) {
                return (new nodes::CopyImplicitCapture(location))->get();
            } else if (capture_type == 1) {
                return (new nodes::RefImplicitCapture(location))->get();
            } else {
                return (new nodes::ConstRefImplicitCapture(location))->get();
            }
        }

    }

    NodePtr parse_closure(parser_state& state) {
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
        state.eat(Pipe,"'|' to end an argument list", error::ErrorCode::UnexpectedEoF);
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
            state.eat(RightBracket,"']' to end a capture list", error::ErrorCode::UnexpectedEoF);
        }
        std::optional<NodePtr> return_type;
        if (front.ty == ThickArrow) {
            state.eatAny();
            return_type = parse_expression(state);
        }
        NodePtr body = parse_block_statement(state);
        return (new nodes::Closure(location,arguments,captures,return_type,body))->get();
    }

    NodePtr parse_function_type(parser_state& state, Coordinate location) {
        auto arg_types = parse_call_list(state,RightParen);
        state.eat(ThickArrow,"a '=>' for signalling the return type of a function type",error::ErrorCode::ExpectedReturnSpecifier);
        NodePtr return_type = parse_expression(state);
        FlagSet fn_flags = parse_flags(state);
        return (new nodes::FunctionType(location,arg_types,return_type,fn_flags))->get();
    }

    NodePtr parse_anon_function_or_type(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        if (state.peek_skip_nl().ty == LeftParen) {
            state.eatAny();
            return parse_function_type(state,location);
        } else {
            NodeList arguments{};
            auto front = state.peek_skip_nl();
            while (front.ty != ThickArrow) {
                if (front.ty == Comma) {
                    state.eatAny();
                }
                else {
                    arguments.push_back(parse_argument(state));
                }
                front = state.peek_skip_nl();
            }
            state.eatAny();
            NodePtr return_type = parse_expression(state);
            FlagSet fn_flags = parse_flags(state);
            NodePtr body = parse_block_statement(state);
            return (new nodes::AnonymousFunction(location,arguments,return_type,fn_flags,body))->get();
        }
    }

    NodePtr parse_generator_type(parser_state& state, Coordinate location) {
        auto arg_types = parse_call_list(state,RightParen);
        state.eat(ThickArrow,"a '=>' for signalling the return type of a generator type",error::ErrorCode::ExpectedReturnSpecifier);
        NodePtr return_type = parse_expression(state);
        FlagSet fn_flags = parse_flags(state);
        return (new nodes::GeneratorType(location,arg_types,return_type,fn_flags))->get();
    }

    NodePtr parse_anon_generator_or_type(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        if (state.peek_skip_nl().ty == LeftParen) {
            state.eatAny();
            return parse_generator_type(state,location);
        } else {
            NodeList arguments{};
            auto front = state.peek_skip_nl();

            while (front.ty != ThickArrow) {
                if (front.ty == Comma) {
                    state.eatAny();
                }
                else {
                    arguments.push_back(parse_argument(state));
                }
                front = state.peek_skip_nl();
            }
            state.eatAny();
            NodePtr return_type = parse_expression(state);
            FlagSet fn_flags = parse_flags(state);
            NodePtr body = parse_block_statement(state);
            return (new nodes::AnonymousGenerator(location,arguments,return_type,fn_flags,body))->get();
        }
    }

    NodePtr parse_block(parser_state& state) {
        auto location = state.peek_skip_nl().location;
        state.eatAny();
        NodeList nodes{};
        auto front = state.peek_skip_nl();
        while (front.ty != RightBrace) {
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
        state.eatAny();
        return (new nodes::Block(location,nodes))->get();
    }

    NodePtr parse_primary_base(parser_state& state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
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
            case UnsignedIntType:
            {
                auto sub = front.value.substr(1);
                std::uint64_t size;
                std::stringstream ss;
                ss << sub;
                ss >> size;
                state.eatAny();
                return (new nodes::UnsignedIntType(location,size))->get();
            }
            case SignedIntType:
            {
                auto sub = front.value.substr(1);
                std::uint64_t size;
                std::stringstream ss;
                ss << sub;
                ss >> size;
                state.eatAny();
                return (new nodes::SignedIntType(location,size))->get();
            }
            case Ampersand:
            {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::AddressOf(location,child))->get();
            }
            case Star:
            {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::Reference(location,child,false))->get();
            }
            case Identifier:
            {
                state.eatAny();
                return (new nodes::ValueReference(location,static_cast<std::string>(front.value)))->get();
            }
            case BuiltinReference:
            {
                state.eatAny();
                return (new nodes::BuiltinReference(location,static_cast<std::string>(front.value[0] == '$' ? front.value.substr(1) : front.value)))->get();
            }
            case ConstPointer:
            {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::Reference(location,child,true))->get();
            }
            case Float32:
            {
                state.eatAny();
                return (new nodes::Float32(location))->get();
            }
            case Float64:
            {
                state.eatAny();
                return (new nodes::Float64(location))->get();
            }
            case Dash:
            {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::UnaryMinus(location,child))->get();
            }
            case Plus:
            {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::UnaryPlus(location,child))->get();
            }
            case Not:
            {
                state.eatAny();
                auto child = parse_expression(state);
                return (new nodes::Not(location,child))->get();
            }
            case Bool:
            {
                state.eatAny();
                return (new nodes::Bool(location))->get();
            }
            case True:
            {
                state.eatAny();
                return (new nodes::True(location))->get();
            }
            case False:
            {
                state.eatAny();
                return (new nodes::False(location))->get();
            }
            case Void:
            {
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
            case Tuple:
            {
                state.eatAny();
                auto args = parse_call_list(state,RightParen);
                return (new nodes::TupleLiteral(location,args))->get();
            }
            case Array:
            {
                state.eatAny();
                auto args = parse_call_list(state,RightBracket);
                return (new nodes::ArrayLiteral(location,args))->get();
            }
            case Dot:
            {
                state.eatAny();
                auto id = static_cast<std::string>(state.peek_skip_nl().value);
                state.eat(Identifier,"an identifier for an enum literal",error::ErrorCode::ExpectedName);
                return (new nodes::EnumLiteral(location,id))->get();
            }
            case Any:
            {
                state.eatAny();
                return (new nodes::AnyType(location))->get();
            }
            case Object:
            {
                state.eatAny();
                return (new nodes::ObjectLiteral(location, parse_object_list(state)))->get();
            }
            case LeftBrace:
            {
                return parse_block(state);
            }
            default:
                std::string not_impl_for = static_cast<std::string>(lexer::name_of(front.ty)) + " (" + static_cast<std::string>(front.value) + ")";
                NOT_IMPL_FOR(not_impl_for)
        }
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

    NodePtr parse_block_statement(parser_state& state) {
        auto front = state.peek_skip_nl();
        auto location = front.location;
        switch (front.ty) {
            case Let:
                return parse_variable_decl(state);
            case Def:
                return parse_definition(state);
            case DoubleThickArrow:
            {
                state.eatAny();
                front = state.peek();
                if (front.ty == NewLine || front.ty == Semicolon || front.ty == EoF) {
                    return (new nodes::EmptyReturn(location))->get();
                } else if (front.ty == None) {
                    state.eatAny();
                    return (new nodes::EmptyReturn(location))->get();
                } else {
                    auto result = parse_expression(state);
                    return (new nodes::Return(location,result))->get();
                }
            }
            case ReversedDoubleThickArrow:
            {
                state.eatAny();
                front = state.peek();
                if (front.ty == NewLine || front.ty == Semicolon || front.ty == EoF) {
                    return (new nodes::EmptyBreak(location))->get();
                } else if (front.ty == None) {
                    state.eatAny();
                    return (new nodes::EmptyBreak(location))->get();
                } else {
                    auto result = parse_expression(state);
                    return (new nodes::Break(location,result))->get();
                }
            }
            case BlockYield:
            {
                state.eatAny();
                front = state.peek();
                auto name = static_cast<std::string>(front.value);
                state.eat(Identifier,"a block name",error::ErrorCode::ExpectedName);
                state.eat(RightParen,"a ')' to close off a named block yield", error::ErrorCode::ExpectedClosingParentheses);
                auto result = parse_expression(state);
                return (new nodes::NamedBreak(location,name,result))->get();
            }
            case Continue:
                state.eatAny();
                return (new nodes::Continue(location))->get();
            case Break:
                state.eatAny();
                return (new nodes::EmptyBreak(location))->get();
            default: {
                auto base = parse_expression(state);
                front = state.peek();
                if (valid_assignment_operator(front.ty)) {
                    state.eatAny();
                    auto value = parse_expression(state);
                    return create_node_from_binary_operator(front,base,value);
                }
                return base;
            }
        }
    }
}