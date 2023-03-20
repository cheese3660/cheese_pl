//
// Created by Lexi Allen on 9/21/2022.
//

#ifndef CHEESE_NODE_H
#define CHEESE_NODE_H

#include <cstdint>
#include <memory>
#include "thirdparty/json.hpp"
#include "Coordinate.h"
#include "math/BigInteger.h"
#include <optional>
#include <iostream>

namespace cheese::parser {
    class Node {
    public:
        cheese::Coordinate location;

        explicit Node(cheese::Coordinate location) : location(location) {}

        [[nodiscard]] virtual nlohmann::json as_json() const = 0;

        [[nodiscard]] virtual bool compare_json(const nlohmann::json &) const = 0;

        virtual ~Node() = default;

        std::shared_ptr<Node> get();
    };

    struct FlagSet {
        std::uint16_t inlin: 1;
        std::uint16_t exter: 1;
        std::uint16_t exp: 1;
        std::uint16_t comptime: 1;
        std::uint16_t pub: 1;
        std::uint16_t priv: 1;
        std::uint16_t mut: 1;
        std::uint16_t entry: 1;
    };
    typedef std::shared_ptr<Node> NodePtr;
    typedef std::vector<NodePtr> NodeList;
    typedef std::map<std::string, NodePtr> NodeDict;


    bool compare_helper(const nlohmann::json &object, std::string name, const NodePtr &value);

    bool implicit_compare_value(const NodePtr &value);


    bool compare_helper(const nlohmann::json &object, std::string name, const std::string &value);

    bool implicit_compare_value(const std::string &value);

    bool compare_helper(const nlohmann::json &object, std::string name, bool value);

    bool implicit_compare_value(bool value);

    bool compare_helper(const nlohmann::json &object, std::string name, const NodeList &value);

    bool implicit_compare_value(const NodeList &value);

    bool compare_helper(const nlohmann::json &object, std::string name, std::uint64_t value);

    bool implicit_compare_value(std::uint64_t value);

    bool compare_helper(const nlohmann::json &object, std::string name, int64_t value);

    bool implicit_compare_value(std::int64_t value);

    bool compare_helper(const nlohmann::json &object, std::string name, double value);

    bool implicit_compare_value(double value);

    bool compare_helper(const nlohmann::json &object, std::string name, char *value);

    bool compare_helper(const nlohmann::json &object, std::string name, const FlagSet &value);

    bool implicit_compare_value(const FlagSet &value);

    bool compare_helper(const nlohmann::json &object, std::string name, const math::BigInteger &value);

    bool implicit_compare_value(const math::BigInteger &value);

    bool compare_helper(const nlohmann::json &object, std::string name, const NodeDict &value);

    bool implicit_compare_value(const NodeDict &value);

    void implicit_compare_check(bool &success);

    template<typename T>
    bool compare_helper(const nlohmann::json &object, std::string name, const std::optional<T> &value) {
        if (value.has_value()) {
            return compare_helper(object, name, value.value());
        } else {
            if (object.contains(name)) {
                return object[name].is_null();
            } else {
                return true;
            }
        }
    }

    template<typename T>
    bool implicit_compare_value(const std::optional<T> &value) {
        return !value.has_value();
    }

    template<typename T>
    void build_json(nlohmann::json &object, std::string name, const T &value) {
        if (implicit_compare_value(value)) return;
        object[name] = value;
    }

    template<>
    void build_json<NodePtr>(nlohmann::json &object, std::string name, const NodePtr &value);

    template<>
    void build_json<NodeList>(nlohmann::json &object, std::string name, const NodeList &value);

    template<>
    void build_json<FlagSet>(nlohmann::json &object, std::string name, const FlagSet &value);

    template<>
    void build_json<math::BigInteger>(nlohmann::json &object, std::string name, const math::BigInteger &value);

    template<typename J>
    void build_json(nlohmann::json &object, std::string name, const std::optional<J> &value) {
        if (implicit_compare_value(value)) return;
        return build_json(object, name, value.value());
    }

    template<>
    void build_json<NodeDict>(nlohmann::json &object, std::string name, const NodeDict &value);

    template<typename T, typename ...Ts>
    void implicit_compare_check(bool &success, T &arg, Ts &... args) {
        if (!implicit_compare_value(arg)) {
            success = false;
            return;
        }
        implicit_compare_check(success, args...);
    }

    template<size_t idx>
    void compare_helper_check(const nlohmann::json &object, bool &success, std::vector<std::string> &names) {
        if (idx != names.size()) success = false;
    }

    template<size_t idx, typename T, typename ...Ts>
    void
    compare_helper_check(const nlohmann::json &object, bool &success, std::vector<std::string> &names, T &arg,
                         Ts &... args) {
        if (idx >= names.size()) {
            success = false;
            return;
        }
        if (!compare_helper(object, names[idx], arg)) {
            success = false;
            return;
        }
        compare_helper_check<idx + 1>(object, success, names, args...);
    }

    template<typename ...T>
    bool
    compare_helper(const nlohmann::json &object, std::string type, std::vector<std::string> arg_names,
                   const T &... args) {
        //Check if everything is implicit if nothing else
        bool success = true;

        if (!object.is_object()) {
            implicit_compare_check(success, args...);
            return success;
        }

        if (!object.contains("type")) return false;
        if (object["type"] != type) return false;
        compare_helper_check<0>(object, success, arg_names, args...);
        return success;
    }

    bool compare_helper(const nlohmann::json &object, std::string type);

    template<size_t idx>
    void build_json_helper(nlohmann::json &target, std::vector<std::string> arg_names) {
    }

    template<size_t idx, typename T, typename... Ts>
    void build_json_helper(nlohmann::json &target, std::vector<std::string> arg_names, T &arg, Ts &... args) {
        if (idx >= arg_names.size()) return;

        build_json(target, arg_names[idx], arg);
        build_json_helper<idx + 1>(target, arg_names, args...);
    }

    template<typename ...Ts>
    nlohmann::json build_json(std::string type, std::vector<std::string> arg_names, Ts &... args) {
        bool all_implicit = true;
        implicit_compare_check(all_implicit, args...);
        if (all_implicit) return type;
        auto result = nlohmann::json::object();
        result["type"] = type;
        build_json_helper<0>(result, arg_names, args...);
        return result;
    }
}


//Simple Node Declarations

#define TERMINAL_NODE(N, T) struct N final : public Node { \
    explicit N(Coordinate location) : Node(location) {}     \
    [[nodiscard]] nlohmann::json as_json() const override {             \
        return build_json(T,{});                   \
    }                                                \
    [[nodiscard]] bool compare_json(const nlohmann::json& o) const override {   \
        return compare_helper(o,T);               \
    }                                              \
    ~N() override = default;                       \
};


#define SINGLE_MEMBER_NODE(N, T, C, CN) struct N final : public Node { \
    C CN;                                                 \
    N(Coordinate location, C CN) : Node(location), CN(std::move(CN)) {} \
    [[nodiscard]] nlohmann::json as_json() const override {                       \
        return build_json(T,{# CN},CN);                       \
    }                                                         \
    [[nodiscard]] bool compare_json(const nlohmann::json& o) const override {            \
        return compare_helper(o,T,{# CN},CN);                       \
    }                                                         \
    ~N() override = default;\
};

#define SINGLE_CHILD_NODE(N, T) SINGLE_MEMBER_NODE(N,T,NodePtr,child)

#define POSSIBLY_CONST_TYPE_NODE(N, T) struct N final : public Node { \
    NodePtr child;                                              \
    bool constant;                                                            \
    N(Coordinate location, NodePtr child, bool constant = false) : Node(location), child(std::move(child)), constant(constant) {} \
    [[nodiscard]] nlohmann::json as_json() const override {                       \
        return build_json(T,{"child","constant"},child,constant);                       \
    }                                                         \
    [[nodiscard]] bool compare_json(const nlohmann::json& o) const override {            \
        return compare_helper(o,T,{"child","constant"},child,constant);                       \
    }                                                         \
    ~N() override = default;\
};

#define DOUBLE_MEMBER_NODE(N, T, C1, C1N, C2, C2N) struct N final : public Node { \
    C1 C1N;                                                         \
    C2 C2N;                                                         \
    N(Coordinate location, C1 C1N, C2 C2N) : Node(location), C1N(std::move(C1N)), C2N(std::move(C2N)) {} \
    [[nodiscard]] nlohmann::json as_json() const override {                                \
        return build_json(T,{# C1N,# C2N},C1N,C2N);                  \
    }                                                                  \
    [[nodiscard]] bool compare_json(const nlohmann::json& o) const override {                      \
        return compare_helper(o,T,{# C1N, # C2N},C1N,C2N);                                                               \
    }\
};


#define BINARY_NODE(N, T) DOUBLE_MEMBER_NODE(N, T, NodePtr, lhs, NodePtr, rhs)

#define JSON_FUNCS(...) \
[[nodiscard]] nlohmann::json as_json() const override \
{                                                                             \
    return build_json(__VA_ARGS__);                                               \
} \
[[nodiscard]] bool compare_json(const nlohmann::json& o) const override       \
{                                                                             \
    return compare_helper(o, __VA_ARGS__);                                        \
}

#endif //CHEESE_NODE_H
