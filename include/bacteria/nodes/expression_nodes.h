//
// Created by Lexi Allen on 4/23/2023.
//

#ifndef CHEESE_EXPRESSION_NODES_H
#define CHEESE_EXPRESSION_NODES_H

#include <optional>
#include <utility>
#include "bacteria/BacteriaNode.h"

namespace cheese::bacteria::nodes {
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
            add_indentation(ss, depth + 1);
            ss << body->get_textual_representation(depth + 1);
            if (els.has_value()) {
                ss << '\n';
                add_indentation(ss, depth);
                ss << "else\n";
                add_indentation(ss, depth + 1);
                ss << els.value()->get_textual_representation(depth + 1);
            }
            return ss.str();
        }
    };

    struct Nop : BacteriaNode {
        Nop(const Coordinate &location) : BacteriaNode(location) {

        }

        ~Nop() override = default;

        std::string get_textual_representation(int depth) override {
            return "/* elided */";
        }
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
    };

    struct ValueReference : BacteriaNode {

        std::string name;

        ValueReference(const Coordinate &location, std::string name) : BacteriaNode(location), name(name) {}

        ~ValueReference() override = default;

        std::string get_textual_representation(int depth) override {
            return name;
        }
    };

    struct CastNode : BacteriaNode {
        CastNode(const Coordinate &location, BacteriaPtr lhs, TypePtr rhs) : BacteriaNode(location),
                                                                             lhs(std::move(lhs)), rhs(rhs) {}

        BacteriaPtr lhs;
        TypePtr rhs;

        ~CastNode() override = default;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << '(' << lhs->get_textual_representation(depth) << " @ " << rhs->to_string() << ')';
            return ss.str();
        }
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
    };

    struct BinaryNode : BacteriaNode {

        BinaryNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BacteriaNode(location),
                                                                                   lhs(std::move(lhs)),
                                                                                   rhs(std::move(rhs)) {}

        virtual const char *get_operator() = 0;

        std::string get_textual_representation(int depth) override final {
            std::stringstream ss{};
            ss << '(' << lhs->get_textual_representation(depth) << ' ' << get_operator() << ' '
               << rhs->get_textual_representation(depth) << ')';
            return ss.str();
        }

        BacteriaPtr lhs;
        BacteriaPtr rhs;
    };

    struct NotEqualNode : BinaryNode {
        NotEqualNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~NotEqualNode() override = default;

        const char *get_operator() override {
            return "!=";
        }
    };

    struct EqualToNode : BinaryNode {
        EqualToNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                               std::move(lhs),
                                                                                               std::move(rhs)) {}

        ~EqualToNode() override = default;

        const char *get_operator() override {
            return "==";
        }
    };

    struct MultiplyNode : BinaryNode {
        MultiplyNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~MultiplyNode() override = default;

        const char *get_operator() override {
            return "*";
        }
    };

    struct SubtractNode : BinaryNode {
        SubtractNode(const Coordinate &location, BacteriaPtr lhs, BacteriaPtr rhs) : BinaryNode(location,
                                                                                                std::move(lhs),
                                                                                                std::move(rhs)) {}

        ~SubtractNode() override = default;

        const char *get_operator() override {
            return "-";
        }
    };
}
#endif //CHEESE_EXPRESSION_NODES_H
