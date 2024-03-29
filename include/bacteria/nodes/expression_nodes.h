//
// Created by Lexi Allen on 4/23/2023.
//

#ifndef CHEESE_EXPRESSION_NODES_H
#define CHEESE_EXPRESSION_NODES_H

#include <optional>
#include <utility>
#include "bacteria/BacteriaNode.h"
#include "bacteria/nodes/receiver_nodes.h"
#include "stringutil.h"
#include <sstream>

namespace cheese::bacteria::nodes {

    //TODO: remove needless parentheses around expressions

    struct FunctionImport : BacteriaNode {
        FunctionImport(Coordinate location, std::string n, TypeList args, bacteria::TypePtr rt)
                : BacteriaNode(location), name(std::move(n)), arguments(std::move(args)), return_type(rt) {

        }

        std::string name;
        TypeList arguments;
        bacteria::TypePtr return_type;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "fn " << name << " ";
            for (int i = 0; i < arguments.size(); i++) {
                auto &argument = arguments[i];
                ss << argument->to_string();
                if (i < arguments.size() - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << "=> " << return_type->to_string() << " import";
            return ss.str();
        }

        ~FunctionImport() override = default;

        JSON_FUNCS("function", { "name", "arguments", "return_type" }, name, arguments,
                   return_type->to_string())

        void lower_top_level(BacteriaContext *ctx) override;

        void gen_protos(BacteriaContext *ctx) override;
    };

    struct Return : BacteriaNode {
        std::optional<BacteriaPtr> retVal;

        Return(const Coordinate &location, BacteriaPtr retVal) : BacteriaNode(location),
                                                                 retVal(std::move(retVal)) {}

        Return(const Coordinate &location) : BacteriaNode(location), retVal() {}

        ~Return() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "return";
            if (retVal.has_value()) {
                ss << ' ' << retVal.value()->get_textual_representation(depth);
            }
            return ss.str();
        }

        JSON_FUNCS("return", { "value" }, retVal)

        void lower_scope_level(ScopeContext &ctx) override;
    };

    struct If : BacteriaNode {
        If(const Coordinate &location, BacteriaPtr condition, BacteriaPtr body,
           BacteriaPtr &&els) : BacteriaNode(location), condition(std::move(condition)), body(std::move(body)),
                                els(std::move(els)) {}

        If(const Coordinate &location, BacteriaPtr condition, BacteriaPtr body) : BacteriaNode(location),
                                                                                  condition(std::move(condition)),
                                                                                  body(std::move(body)), els() {}

        BacteriaPtr condition;
        BacteriaPtr body;
        std::optional<BacteriaPtr> els;

        ~If() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "if " << condition->get_textual_representation(depth) << '\n';
            auto d = dynamic_cast<UnnamedBlock *>(body.get()) ? 0 : 1;
            add_indentation(ss, depth + d);
            ss << body->get_textual_representation(depth + d);
            if (els.has_value()) {
                ss << '\n';
                add_indentation(ss, depth);
                if (dynamic_cast<If *>(els.value().get())) {
                    ss << "else ";
                    ss << els.value()->get_textual_representation(depth);
                } else {
                    ss << "else\n";
                    auto d2 = dynamic_cast<UnnamedBlock *>(els.value().get()) ? 0 : 1;
                    add_indentation(ss, depth + d2);
                    ss << els.value()->get_textual_representation(depth + d2);
                }
            }
            return ss.str();
        }

        JSON_FUNCS("if", { "condition", "body", "else" }, condition, body, els)

        void lower_scope_level(ScopeContext &ctx) override;
    };

    struct While : BacteriaNode {
        While(const Coordinate &location, BacteriaPtr condition, BacteriaPtr body,
              BacteriaPtr &&els) : BacteriaNode(location), condition(std::move(condition)), body(std::move(body)),
                                   els(std::move(els)) {}

        While(const Coordinate &location, BacteriaPtr condition, BacteriaPtr body) : BacteriaNode(location),
                                                                                     condition(std::move(condition)),
                                                                                     body(std::move(body)), els() {}

        BacteriaPtr condition;
        BacteriaPtr body;
        std::optional<BacteriaPtr> els;

        ~While() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "while " << condition->get_textual_representation(depth) << '\n';
            auto d = dynamic_cast<UnnamedBlock *>(body.get()) ? 0 : 1;
            add_indentation(ss, depth + d);
            ss << body->get_textual_representation(depth + d);
            if (els.has_value()) {
                ss << '\n';
                add_indentation(ss, depth);
                if (dynamic_cast<If *>(els.value().get())) {
                    ss << "else ";
                    ss << els.value()->get_textual_representation(depth);
                } else {
                    ss << "else\n";
                    auto d2 = dynamic_cast<UnnamedBlock *>(els.value().get()) ? 0 : 1;
                    add_indentation(ss, depth + d2);
                    ss << els.value()->get_textual_representation(depth + d2);
                }
            }
            return ss.str();
        }

        JSON_FUNCS("while", { "condition", "body", "else" }, condition, body, els)

        void lower_scope_level(ScopeContext &ctx) override;
    };

    struct Nop : BacteriaNode {
        Nop(const Coordinate &location) : BacteriaNode(location) {

        }

        ~Nop() override = default;

        std::string get_textual_representation(int depth) override {
            return "/* elided */";
        }

        JSON_FUNCS("nop", std::vector<std::string>{})
    };


    struct IntegerLiteral : BacteriaNode {
        IntegerLiteral(const Coordinate &location, const math::BigInteger &value, TypePtr type) : BacteriaNode(
                location), value(value), type(std::move(type)) {
        }

        math::BigInteger value;
        TypePtr type;

        ~IntegerLiteral() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << '(' << static_cast<std::string>(value) << " @ " << type->to_string() << ')';
            return ss.str();
        }

        JSON_FUNCS("integer", { "value", "ty" }, value, (type->to_string()))

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;;
    };

    struct StringLiteral : BacteriaNode {
        StringLiteral(const Coordinate &location, std::string value, TypePtr type) : BacteriaNode(
                location), value(std::move(value)), type(type) {
        }

        std::string value;
        TypePtr type;

        ~StringLiteral() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << '(' << '"' << stringutil::escape(value) << '"' << " @ " << type->to_string() << ')';
            return ss.str();
        }

        JSON_FUNCS("integer", { "value", "ty" }, value, (type->to_string()))

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        TypePtr get_expr_type(ScopeContext &ctx, BacteriaProgram *program) override;
    };

    struct FloatLiteral : BacteriaNode {
        FloatLiteral(const Coordinate &location, double value, TypePtr type) : BacteriaNode(
                location), value(value), type(std::move(type)) {
        }

        double value;
        TypePtr type;

        ~FloatLiteral() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << '(' << value << " @ " << type->to_string() << ')';
            return ss.str();
        }

        JSON_FUNCS("float", { "value", "ty" }, value, (type->to_string()));
    };

    struct ComplexLiteral : BacteriaNode {
        ComplexLiteral(const Coordinate &location, double a, double b, TypePtr type) : BacteriaNode(
                location), a(a), b(b), type(std::move(type)) {
        }

        double a;
        double b;
        TypePtr type;

        ~ComplexLiteral() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << '(' << a << "+" << b << "I @ " << type->to_string() << ')';
            return ss.str();
        }

        JSON_FUNCS("complex", { "a", "b", "ty" }, a, b, (type->to_string()));
    };

    struct ValueReference : BacteriaNode {

        std::string name;

        ValueReference(const Coordinate &location, std::string name) : BacteriaNode(location), name(name) {}

        ~ValueReference() override = default;

        std::string get_textual_representation(int depth) override {
            return name;
        }

        JSON_FUNCS("value", { "name" }, name)

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;

        llvm::Value *lower_address(ScopeContext &ctx) override;
    };

    struct CastNode : BacteriaNode {
        CastNode(const Coordinate &location, BacteriaPtr lhs, TypePtr rhs) : BacteriaNode(location),
                                                                             lhs(std::move(lhs)), rhs(rhs) {}

        BacteriaPtr lhs;
        TypePtr rhs;

        ~CastNode() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << lhs->get_textual_representation(depth) << " @ " << rhs->to_string();
            return ss.str();
        }

        JSON_FUNCS("cast", { "value", "ty" }, lhs, (rhs->to_string()))

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;
    };

    struct NormalCallNode : BacteriaNode {
        NormalCallNode(const Coordinate &location, std::string function,
                       std::vector<BacteriaPtr> arguments) : BacteriaNode(location), function(function),
                                                             arguments(std::move(arguments)) {}

        std::string function;
        std::vector<BacteriaPtr> arguments;

        ~NormalCallNode() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << function << '(';
            for (int i = 0; i < arguments.size(); i++) {
                ss << arguments[i]->get_textual_representation(depth);
                if (i < arguments.size() - 1) {
                    ss << ", ";
                }
            }
            ss << ')';
            return ss.str();
        }

        JSON_FUNCS("call", { "function", "arguments" }, function, arguments)

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        void lower_scope_level(ScopeContext &ctx) override;

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;
    };

    struct PointerCallNode : BacteriaNode {
        PointerCallNode(const Coordinate &location, BacteriaPtr function,
                        std::vector<BacteriaPtr> arguments) : BacteriaNode(location), function(std::move(function)),
                                                              arguments(std::move(arguments)) {}

        BacteriaPtr function;
        std::vector<BacteriaPtr> arguments;

        ~PointerCallNode() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "(*" + function->get_textual_representation(depth) + ")" << '(';
            for (int i = 0; i < arguments.size(); i++) {
                ss << arguments[i]->get_textual_representation(depth);
                if (i < arguments.size() - 1) {
                    ss << ", ";
                }
            }
            ss << ')';
            return ss.str();
        }

        JSON_FUNCS("pointer_call", { "function", "arguments" }, function, arguments)
    };

    struct ArrayIndexNode : BacteriaNode {
        ArrayIndexNode(const Coordinate &location, BacteriaPtr array,
                       std::vector<BacteriaPtr> arguments) : BacteriaNode(location), array(std::move(array)),
                                                             arguments(std::move(arguments)) {}

        BacteriaPtr array;
        std::vector<BacteriaPtr> arguments;

        ~ArrayIndexNode() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << array->get_textual_representation(depth) << '[';
            for (int i = 0; i < arguments.size(); i++) {
                ss << arguments[i]->get_textual_representation(depth);
                if (i < arguments.size() - 1) {
                    ss << ", ";
                }
            }
            ss << ']';
            return ss.str();
        }

        JSON_FUNCS("pointer_call", { "array", "arguments" }, array, arguments)

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        llvm::Value *lower_address(ScopeContext &ctx) override;
    };

    struct VariableInitializationNode : BacteriaNode {

        std::string name;
        TypePtr type;
        BacteriaPtr value;
        bool constant;

        VariableInitializationNode(const Coordinate &location, const std::string &name, const TypePtr &type,
                                   BacteriaPtr value, bool constant) : BacteriaNode(location), name(name), type(type),
                                                                       value(std::move(value)), constant(constant) {}

        ~VariableInitializationNode() = default;

        std::string get_textual_representation(int depth) override {
            return name + ": " + type->to_string() + " = " + value->get_textual_representation(depth);
        }

        JSON_FUNCS("init", { "name", "ty", "value", "constant" }, name, type->to_string(), value,
                   std::to_string(constant))

        void lower_scope_level(ScopeContext &ctx) override;;
    };

    struct VariableDefinitionNode : BacteriaNode {
        std::string name;
        TypePtr type;

        VariableDefinitionNode(const Coordinate &location, const std::string &name, const TypePtr &type) : BacteriaNode(
                location), name(name), type(type) {}

        ~VariableDefinitionNode() = default;

        std::string get_textual_representation(int depth) override {
            return name + ": " + type->to_string();
        }

        JSON_FUNCS("def", { "name", "ty" }, name, type->to_string())

        void lower_scope_level(ScopeContext &ctx) override;;
    };

    struct AggregrateObject : BacteriaNode {
        AggregrateObject(const Coordinate &location, const TypePtr &type, std::vector<BacteriaPtr> values)
                : BacteriaNode(location), type(type), values(std::move(values)) {}

        ~AggregrateObject() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "(" << type->to_string() << "){";
            for (int i = 0; i < values.size(); i++) {
                ss << values[i]->get_textual_representation(depth);
                if (i < values.size() - 1) {
                    ss << ", ";
                }
            }
            ss << '}';
            return ss.str();
        }

        TypePtr type;
        std::vector<BacteriaPtr> values;

        JSON_FUNCS("object", { "ty", "values" }, type->to_string(), values);
    };

    struct UnaryMinusNode : BacteriaNode {
        UnaryMinusNode(const Coordinate &location, BacteriaPtr child) : BacteriaNode(location),
                                                                        child(std::move(child)) {}

        ~UnaryMinusNode() override = default;

        std::string get_textual_representation(int depth) override {
            return "(-" + child->get_textual_representation(depth) + ")";
        }

        BacteriaPtr child;

        JSON_FUNCS("unary -", { "child" }, child)
    };


    struct UnaryPlusNode : BacteriaNode {
        UnaryPlusNode(const Coordinate &location, BacteriaPtr child) : BacteriaNode(location),
                                                                       child(std::move(child)) {}

        ~UnaryPlusNode() override = default;

        std::string get_textual_representation(int depth) override {
            return "(+" + child->get_textual_representation(depth) + ")";
        }

        BacteriaPtr child;

        JSON_FUNCS("unary +", { "child" }, child)
    };


    struct ImplicitReferenceNode : BacteriaNode {
        ImplicitReferenceNode(const Coordinate &location, BacteriaPtr child) : BacteriaNode(location),
                                                                               child(std::move(child)) {}

        ~ImplicitReferenceNode() override = default;

        std::string get_textual_representation(int depth) override {
            return "(&&" + child->get_textual_representation(depth) + ")";
        }

        BacteriaPtr child;

        JSON_FUNCS("&&", { "child" }, child)
    };

    struct ReferenceNode : BacteriaNode {
        ReferenceNode(const Coordinate &location, BacteriaPtr child) : BacteriaNode(location),
                                                                       child(std::move(child)) {}

        ~ReferenceNode() override = default;

        std::string get_textual_representation(int depth) override {
            return "(&" + child->get_textual_representation(depth) + ")";
        }

        BacteriaPtr child;

        JSON_FUNCS("&", { "child" }, child)
    };

    struct ObjectSubscriptNode : BacteriaNode {
        ObjectSubscriptNode(const Coordinate &location, BacteriaPtr child, int index) : BacteriaNode(location),
                                                                                        child(std::move(child)),
                                                                                        index(index) {}

        ~ObjectSubscriptNode() override = default;

        std::string get_textual_representation(int depth) override {
            return "(" + child->get_textual_representation(depth) + "." + std::to_string(index) + ")";
        }


        BacteriaPtr child;
        int index;

        JSON_FUNCS(".", { "child", "index" }, child, static_cast<int64_t>(index))
    };

    struct ReferenceSubscriptNode : BacteriaNode {
        ReferenceSubscriptNode(const Coordinate &location, BacteriaPtr child, int index) : BacteriaNode(location),
                                                                                           child(std::move(child)),
                                                                                           index(index) {}

        ~ReferenceSubscriptNode() override = default;

        std::string get_textual_representation(int depth) override {
            return "(" + child->get_textual_representation(depth) + "->" + std::to_string(index) + ")";
        }


        BacteriaPtr child;
        int index;

        JSON_FUNCS("->", { "child", "index" }, child, static_cast<int64_t>(index))
    };

    struct BinaryNode : BacteriaNode {

        BinaryNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BacteriaNode(location),
                                                                                   lhs(std::move(lhs)),
                                                                                   rhs(std::move(rhs)) {}

        virtual const char *get_operator() const = 0;

        std::string get_textual_representation(int depth) override final {
            std::stringstream ss{};
            ss << '(' << lhs->get_textual_representation(depth) << ' ' << get_operator() << ' '
               << rhs->get_textual_representation(depth) << ')';
            return ss.str();
        }

        BacteriaPtr lhs;
        BacteriaPtr rhs;

        JSON_FUNCS(get_operator(), { "lhs", "rhs" }, lhs, rhs);
    };

    struct NotEqualNode : BinaryNode {
        NotEqualNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~NotEqualNode() override = default;

        const char *get_operator() const override {
            return "!=";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;
    };

    struct EqualToNode : BinaryNode {
        EqualToNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                               std::move(lhs),
                                                                                               std::move(rhs)) {}

        ~EqualToNode() override = default;

        const char *get_operator() const override {
            return "==";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;
    };

    struct LesserThanNode : BinaryNode {
        LesserThanNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                  std::move(lhs),
                                                                                                  std::move(rhs)) {}

        ~LesserThanNode() override = default;

        const char *get_operator() const override {
            return "<";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;

    };

    struct MultiplyNode : BinaryNode {
        MultiplyNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~MultiplyNode() override = default;

        const char *get_operator() const override {
            return "*";
        }
    };

    struct SubtractNode : BinaryNode {
        SubtractNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~SubtractNode() override = default;

        const char *get_operator() const override {
            return "-";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;
    };

    struct ModulusNode : BinaryNode {
        ModulusNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                               std::move(lhs),
                                                                                               std::move(rhs)) {}

        ~ModulusNode() override = default;

        const char *get_operator() const override {
            return "%";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;
    };

    struct DivisionNode : BinaryNode {
        DivisionNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~DivisionNode() override = default;

        const char *get_operator() const override {
            return "/";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;
    };

    struct OrNode : BinaryNode {
        OrNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                          std::move(lhs),
                                                                                          std::move(rhs)) {}

        ~OrNode() override = default;

        const char *get_operator() const override {
            return "or";
        }
    };

    struct AdditionNode : BinaryNode {
        AdditionNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~AdditionNode() override = default;

        const char *get_operator() const override {
            return "+";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;
    };

    struct GreaterEqualNode : BinaryNode {
        GreaterEqualNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                    std::move(lhs),
                                                                                                    std::move(rhs)) {}

        ~GreaterEqualNode() override = default;

        const char *get_operator() const override {
            return ">=";
        }

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;

        TypePtr get_expr_type(ScopeContext &ctx, nodes::BacteriaProgram *program) override;
    };

    struct MutationNode : BinaryNode {
        MutationNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~MutationNode() override = default;

        const char *get_operator() const override {
            return "=";
        }

        void lower_scope_level(ScopeContext &ctx) override;
    };

//    struct WriteNode : BinaryNode {
//        WriteNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
//                                                                                             std::move(lhs),
//                                                                                             std::move(rhs)) {}
//
//        ~WriteNode() override = default;
//
//        const char *get_operator() const override {
//            return "<-";
//        }
//    };
}
#endif //CHEESE_EXPRESSION_NODES_H
