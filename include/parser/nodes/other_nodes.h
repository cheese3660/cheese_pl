//
// Created by Lexi Allen on 11/11/2022.
//

#ifndef CHEESE_OTHER_NODES_H
#define CHEESE_OTHER_NODES_H

#include <utility>

#include "../Node.h"
#include "NotImplementedException.h"
#include <optional>

namespace cheese::parser::nodes {

    DOUBLE_MEMBER_NODE(TupleCall, "tuple_call", NodePtr, object, NodeList, args)

    DOUBLE_MEMBER_NODE(ArrayCall, "array_call", NodePtr, object, NodeList, args)

    DOUBLE_MEMBER_NODE(NamedBlock, "named_block", std::string, name, NodeList, children)

    DOUBLE_MEMBER_NODE(NamedBreak, "named_break", std::string, name, NodePtr, value)

    DOUBLE_MEMBER_NODE(ObjectCall, "object_call", NodePtr, object, NodeList, args)

    DOUBLE_MEMBER_NODE(ErrorNode, "error", std::uint64_t, error_code, std::string, message)

    DOUBLE_MEMBER_NODE(Match, "match", NodePtr, value, NodeList, arms)

    DOUBLE_MEMBER_NODE(MatchRange, "match_range", NodePtr, begin, NodePtr, end)

    DOUBLE_MEMBER_NODE(DestructuringMatchArm, "destructuring_match_arm", NodeList, matches, std::optional<NodePtr>,
                       store)

    DOUBLE_MEMBER_NODE(MatchEnumStructure, "match_enum_structure", NodeDict, children, std::string, enum_identifier)

    DOUBLE_MEMBER_NODE(MatchEnumTuple, "match_enum_tuple", NodeList, children, std::string, enum_identifier)

    DOUBLE_MEMBER_NODE(Enum, "enum", std::optional<NodePtr>, containing_type, NodeList, children)

    DOUBLE_MEMBER_NODE(FieldLiteral, "field_literal", std::string, name, NodePtr, value)

    //All the binary operators
    BINARY_NODE(Subscription, "subscript")

    BINARY_NODE(Multiplication, "multiply")

    BINARY_NODE(Division, "divide")

    BINARY_NODE(Modulus, "modulate")

    BINARY_NODE(Addition, "add")

    BINARY_NODE(Subtraction, "subtract")

    BINARY_NODE(LeftShift, "shift_left")

    BINARY_NODE(RightShift, "shift_right")

    BINARY_NODE(LesserThan, "lesser")

    BINARY_NODE(GreaterThan, "greater")

    BINARY_NODE(LesserEqual, "lesser_equal")

    BINARY_NODE(GreaterEqual, "greater_equal")

    BINARY_NODE(EqualTo, "equal")

    BINARY_NODE(NotEqualTo, "not_equal")

    BINARY_NODE(And, "and")

    BINARY_NODE(Or, "or")

    BINARY_NODE(Xor, "xor")

    BINARY_NODE(Combination, "combine")

    BINARY_NODE(Reassignment, "reassign")

    BINARY_NODE(Assignment, "assign")

    BINARY_NODE(SumAssignment, "add_assign")

    BINARY_NODE(DifferenceAssignment, "subtract_assign")

    BINARY_NODE(MultiplicationAssignment, "multiply_assign")

    BINARY_NODE(DivisionAssignment, "divide_assign")

    BINARY_NODE(ModulusAssignment, "modulate_assign")

    BINARY_NODE(LeftShiftAssignment, "shift_left_assign")

    BINARY_NODE(RightShiftAssignment, "shift_right_assign")

    BINARY_NODE(AndAssignment, "and_assign")

    BINARY_NODE(OrAssignment, "or_assign")

    BINARY_NODE(XorAssignment, "xor_assign")

    BINARY_NODE(IsType, "is")

    BINARY_NODE(DynamicCast, "dynamic_cast")

    BINARY_NODE(Cast, "cast")

    BINARY_NODE(Range, "range")


    struct Field final : public Node {
        std::optional<std::string> name;
        NodePtr type;
        FlagSet flags;

        Field(Coordinate location, std::optional<std::string> name, NodePtr type, FlagSet flags) :
                Node(location),
                name(std::move(name)),
                type(std::move(type)),
                flags(std::move(flags)) {
        }

        [[nodiscard]] nlohmann::json as_json() const override {
            return build_json("field", {"name", "field_type", "flags"}, name, type, flags);
        }

        [[nodiscard]] bool compare_json(const nlohmann::json &json) const override {
            return compare_helper(json, "field", {"name", "field_type", "flags"}, name, type, flags);
        }

        ~Field() override = default;

    };

    struct Argument final : public Node {
        std::optional<std::string> name;
        NodePtr type;
        bool comptime;

        Argument(Coordinate location, std::optional<std::string> name, NodePtr type, bool comptime) :
                Node(location),
                name(std::move(name)),
                type(std::move(type)),
                comptime(std::move(comptime)) {
        }

        JSON_FUNCS("arg", { "name", "arg_type", "comptime" }, name, type, comptime)

        ~Argument() override = default;
    };


    struct Import final : public Node {
        std::string path;
        std::string name;

        Import(Coordinate location, std::string path, std::string name) : Node(location), path(std::move(path)),
                                                                          name(std::move(name)) {}

        [[nodiscard]] nlohmann::json as_json() const override;

        [[nodiscard]] bool compare_json(const nlohmann::json &) const override;

        ~Import() override = default;
    };

    struct Structure final : public Node {
        bool is_tuple;
        NodeList interfaces;
        NodeList children;

        Structure(Coordinate location, NodeList interfaces, NodeList children, bool is_tuple) : Node(location),
                                                                                                interfaces(std::move(
                                                                                                        interfaces)),
                                                                                                children(std::move(
                                                                                                        children)),
                                                                                                is_tuple(is_tuple) {}

        [[nodiscard]] nlohmann::json as_json() const override;

        [[nodiscard]] bool compare_json(const nlohmann::json &) const override;

        ~Structure() override = default;
    };


    struct FunctionPrototype final : public Node {
        std::string name;
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;

        FunctionPrototype(Coordinate location, std::string name, NodeList arguments, NodePtr return_type, FlagSet flags)
                :
                Node(location),
                name(std::move(name)),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)) {}

        JSON_FUNCS("function_prototype", { "name", "args", "return_type", "flags" }, name, arguments, return_type,
                   flags)

        ~FunctionPrototype() override = default;
    };

    struct FunctionImport final : public Node {
        std::string name;
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;

        FunctionImport(Coordinate location, std::string name, NodeList arguments, NodePtr return_type, FlagSet flags) :
                Node(location),
                name(std::move(name)),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)) {}

        JSON_FUNCS("function_import", { "name", "args", "return_type", "flags" }, name, arguments, return_type, flags)

        ~FunctionImport() override = default;
    };


    struct Function final : public Node {
        std::string name;
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;
        NodePtr body;

        Function(Coordinate location, std::string name, NodeList arguments, NodePtr return_type, FlagSet flags,
                 NodePtr body) :
                Node(location),
                name(std::move(name)),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)),
                body(std::move(body)) {}

        JSON_FUNCS("function", { "name", "args", "return_type", "flags", "body" }, name, arguments, return_type, flags,
                   body)

        ~Function() override = default;
    };

    struct Operator final : public Node {
        std::string op;
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;
        NodePtr body;

        Operator(Coordinate location, std::string op, NodeList arguments, NodePtr return_type, FlagSet flags,
                 NodePtr body) :
                Node(location),
                op(std::move(op)),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)),
                body(std::move(body)) {}

        JSON_FUNCS("operator", { "operator", "args", "return_type", "flags", "body" }, op, arguments, return_type,
                   flags,
                   body)

        ~Operator() override = default;
    };


    struct GeneratorPrototype final : public Node {
        std::string name;
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;

        GeneratorPrototype(Coordinate location, std::string name, NodeList arguments, NodePtr return_type,
                           FlagSet flags) :
                Node(location),
                name(std::move(name)),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)) {}

        JSON_FUNCS("generator_prototype", { "name", "args", "return_type", "flags" }, name, arguments, return_type,
                   flags)

        ~GeneratorPrototype() override = default;
    };

    struct GeneratorImport final : public Node {
        std::string name;
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;

        GeneratorImport(Coordinate location, std::string name, NodeList arguments, NodePtr return_type, FlagSet flags) :
                Node(location),
                name(std::move(name)),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)) {}

        JSON_FUNCS("generator_import", { "name", "args", "return_type", "flags" }, name, arguments, return_type, flags)

        ~GeneratorImport() override = default;
    };

    struct Generator final : public Node {
        std::string name;
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;
        NodePtr body;

        Generator(Coordinate location, std::string name, NodeList arguments, NodePtr return_type, FlagSet flags,
                  NodePtr body) :
                Node(location),
                name(std::move(name)),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)),
                body(std::move(body)) {}

        JSON_FUNCS("generator", { "name", "args", "return_type", "flags", "body" }, name, arguments, return_type, flags,
                   body)

        ~Generator() override = default;
    };

    struct VariableDeclaration final : public Node {
        NodePtr def;
        NodePtr value;

        VariableDeclaration(Coordinate location, NodePtr def, NodePtr value) :
                Node(location),
                def(std::move(def)),
                value(std::move(value)) {}

        JSON_FUNCS("var_decl", { "def", "value" }, def, value)

        ~VariableDeclaration() override = default;
    };

    struct VariableDefinition final : public Node {
        std::string name;
        std::optional<NodePtr> type;
        FlagSet flags;

        VariableDefinition(Coordinate location, std::string name, std::optional<NodePtr> type, FlagSet flags) :
                Node(location),
                name(std::move(name)),
                type(std::move(type)),
                flags(std::move(flags)) {}

        JSON_FUNCS("var_def", { "name", "var_type", "flags" }, name, type, flags)
    };

    struct Closure final : public Node {
        NodeList args;
        NodeList captures;
        std::optional<NodePtr> return_type;
        NodePtr body;

        Closure(Coordinate location, NodeList args, NodeList captures, std::optional<NodePtr> return_type, NodePtr body)
                :
                Node(location),
                args(std::move(args)),
                captures(std::move(captures)),
                return_type(std::move(return_type)),
                body(std::move(body)) {}

        JSON_FUNCS("closure", { "args", "captures", "return_type", "body" }, args, captures, return_type, body)
    };

    struct AnonymousFunction final : public Node {
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;
        NodePtr body;

        AnonymousFunction(Coordinate location, NodeList arguments, NodePtr return_type, FlagSet flags, NodePtr body) :
                Node(location),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)),
                body(std::move(body)) {}

        JSON_FUNCS("anonymous_function", { "args", "return_type", "flags", "body" }, arguments, return_type, flags,
                   body)

        ~AnonymousFunction() override = default;
    };

    struct AnonymousGenerator final : public Node {
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;
        NodePtr body;

        AnonymousGenerator(Coordinate location, NodeList arguments, NodePtr return_type, FlagSet flags, NodePtr body) :
                Node(location),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)),
                body(std::move(body)) {}

        JSON_FUNCS("anonymous_generator", { "args", "return_type", "flags", "body" }, arguments, return_type, flags,
                   body)

        ~AnonymousGenerator() override = default;
    };

    struct FunctionType final : public Node {
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;

        FunctionType(Coordinate location, NodeList arguments, NodePtr return_type, FlagSet flags) :
                Node(location),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)) {}


        JSON_FUNCS("function_type", { "args", "return_type", "flags" }, arguments, return_type, flags)

        ~FunctionType() override = default;
    };

    struct GeneratorType final : public Node {
        NodeList arguments;
        NodePtr return_type;
        FlagSet flags;

        GeneratorType(Coordinate location, NodeList arguments, NodePtr return_type, FlagSet flags) :
                Node(location),
                arguments(std::move(arguments)),
                return_type(std::move(return_type)),
                flags(std::move(flags)) {}

        JSON_FUNCS("generator_type", { "args", "return_type", "flags" }, arguments, return_type, flags)

        ~GeneratorType() override = default;
    };

    struct StructureDestructure final : public Node {
        NodeDict children;

        StructureDestructure(Coordinate location, NodeDict children) :
                Node(location),
                children(std::move(children)) {}

        JSON_FUNCS("structure_destructure", { "children" }, children)

        ~StructureDestructure() override = default;
    };

    struct TupleDestructure final : public Node {
        NodeList children;

        TupleDestructure(Coordinate location, NodeList children) :
                Node(location),
                children(std::move(children)) {}

        JSON_FUNCS("tuple_destructure", { "children" }, children)

        ~TupleDestructure() override = default;
    };

    struct ArrayDestructure final : public Node {
        NodeList children;

        ArrayDestructure(Coordinate location, NodeList children) :
                Node(location),
                children(std::move(children)) {}

        JSON_FUNCS("array_destructure", { "children" }, children)

        ~ArrayDestructure() override = default;
    };


    //Same as array destructure, but does a runtime check on size, also looks better
    struct SliceDestructure final : public Node {
        NodeList children;

        SliceDestructure(Coordinate location, NodeList children) :
                Node(location),
                children(std::move(children)) {}

        JSON_FUNCS("slice_destructure", { "children" }, children)

        ~SliceDestructure() override = default;
    };

    struct Destructure final : public Node {
        NodePtr structure;
        NodePtr value;

        Destructure(Coordinate location, NodePtr structure, NodePtr value) :
                Node(location),
                structure(std::move(structure)),
                value(std::move(value)) {}

        JSON_FUNCS("destructure", { "structure", "value" }, structure, value)

        ~Destructure() override = default;
    };

    struct Mixin final : public Node {
        NodePtr structure;
        NodeList arguments; // Used for templated mixins, if this is empty, then its a concrete mixin
        NodeList interfaces;
        NodeList children;

        Mixin(Coordinate location, NodePtr structure, NodeList arguments, NodeList interfaces, NodeList children) :
                Node(location),
                structure(std::move(structure)),
                arguments(std::move(arguments)),
                interfaces(std::move(interfaces)),
                children(std::move(children)) {}


        JSON_FUNCS("mixin", { "structure", "arguments", "interfaces", "children" }, structure, arguments, interfaces,
                   children);

        ~Mixin() override = default;
    };

    struct Interface final : public Node {
        NodeList interfaces;
        NodeList children;
        bool dynamic;

        Interface(Coordinate location, NodeList interfaces, NodeList children, bool dynamic) :
                Node(location),
                interfaces(std::move(interfaces)),
                children(std::move(children)),
                dynamic(dynamic) {}


        JSON_FUNCS("interface", { "interfaces", "children", "dynamic" }, interfaces, children, dynamic);

        ~Interface() override = default;
    };

    struct If final : public Node {
        NodePtr condition;
        std::optional<NodePtr> unwrap;
        NodePtr body;
        std::optional<NodePtr> els;

        If(Coordinate location, NodePtr condition, std::optional<NodePtr> unwrap, NodePtr body,
           std::optional<NodePtr> els) :
                Node(location),
                condition(std::move(condition)),
                unwrap(std::move(unwrap)),
                body(std::move(body)),
                els(std::move(els)) {}


        JSON_FUNCS("if", { "condition", "unwrap", "body", "else" }, condition, unwrap, body, els);

        ~If() override = default;
    };

    struct While final : public Node {
        NodePtr condition;
        NodePtr body;
        std::optional<NodePtr> els;

        While(Coordinate location, NodePtr condition, NodePtr body, std::optional<NodePtr> els) :
                Node(location),
                condition(std::move(condition)),
                body(std::move(body)),
                els(std::move(els)) {}


        JSON_FUNCS("while", { "condition", "body", "else" }, condition, body, els);

        ~While() override = default;
    };

    struct For final : public Node {
        NodePtr capture;
        std::optional<NodePtr> index;
        NodePtr iterable;
        NodeList transformations;
        NodePtr body;
        std::optional<NodePtr> els;

        For(Coordinate location, NodePtr capture, std::optional<NodePtr> index, NodePtr iterable,
            NodeList transformations, NodePtr body, std::optional<NodePtr> els)
                :
                Node(location),
                capture(std::move(capture)),
                index(std::move(index)),
                iterable(std::move(iterable)),
                transformations(std::move(transformations)),
                body(std::move(body)),
                els(std::move(els)) {}

        JSON_FUNCS("for", { "capture", "index", "iterable", "transformations", "body", "else" },
                   capture, index, iterable, transformations, body, els);

    };

    struct MatchArm final : public Node {
        NodeList matches;
        std::optional<NodePtr> store;
        NodePtr body;

        MatchArm(Coordinate location, NodeList matches, std::optional<NodePtr> store, NodePtr body) :
                Node(location),
                matches(std::move(matches)),
                store(std::move(store)),
                body(std::move(body)) {}


        JSON_FUNCS("match_arm", { "matches", "store", "body" }, matches, store, body);

        ~MatchArm() override = default;
    };

    struct EnumMember final : public Node {
        std::string name;
        bool tuple;
        NodeList children;
        std::optional<NodePtr> value;

        EnumMember(Coordinate location, std::string name, bool tuple, NodeList children, std::optional<NodePtr> value) :
                Node(location),
                name(std::move(name)),
                tuple(tuple),
                children(std::move(children)),
                value(std::move(value)) {}


        JSON_FUNCS("enum_constant", { "name", "tuple", "children", "value" }, name, tuple, children, value);

        ~EnumMember() override = default;
    };

    struct ArrayType final : public Node {
        NodeList dimensions;
        NodePtr child;
        bool constant;

        ArrayType(Coordinate location, NodeList dimensions, NodePtr child, bool constant) :
                Node(location),
                dimensions(std::move(dimensions)),
                child(std::move(child)),
                constant(constant) {}


        JSON_FUNCS("array_type", { "dimensions", "child", "constant" }, dimensions, child, constant);

        ~ArrayType() override = default;
    };
}
#endif //CHEESE_OTHER_NODES_H
