//
// Created by Lexi Allen on 9/21/2022.
//

#ifndef CHEESE_NODE_H
#define CHEESE_NODE_H

#include <cstdint>
#include <memory>
#include "thirdparty/json.hpp"
#include "Coordinate.h"
namespace cheese::parser {
    class Node {
    public:
        cheese::Coordinate location;
        Node(cheese::Coordinate location) : location(location) {}
        virtual void nested_display(std::uint32_t nesting) = 0;
        virtual nlohmann::json as_json() = 0;
        virtual bool compare_json(nlohmann::json) = 0;
        virtual ~Node() = default;
        std::shared_ptr<Node> get();
    };
    struct FlagSet {
        std::uint16_t inlin : 1;
        std::uint16_t exter : 1;
        std::uint16_t exp : 1;
        std::uint16_t comptime : 1;
        std::uint16_t pub : 1;
        std::uint16_t priv : 1;
        std::uint16_t cons : 1;
    };
    typedef std::shared_ptr<Node> NodePtr;
    typedef std::vector<NodePtr> NodeList;



    bool compare_helper(nlohmann::json& object, std::string name, NodePtr& value);

    bool implicit_compare_value(NodePtr& value);


    bool compare_helper(nlohmann::json& object, std::string name, std::string& value);
    bool implicit_compare_value(std::string& value);

    bool compare_helper(nlohmann::json& object, std::string name, bool& value);
    bool implicit_compare_value(bool& value);
    bool compare_helper(nlohmann::json& object, std::string name, NodeList& value);
    bool implicit_compare_value(NodeList& value);
    bool compare_helper(nlohmann::json& object, std::string name, std::uint64_t& value);

    bool implicit_compare_value(std::uint64_t& value);
    bool compare_helper(nlohmann::json& object, std::string name, std::int64_t& value);
    bool implicit_compare_value(std::int64_t& value);
    bool compare_helper(nlohmann::json& object, std::string name, double& value);
    bool implicit_compare_value(double& value);

    bool compare_helper(nlohmann::json& object, std::string name, const char* value);

    bool compare_helper(nlohmann::json& object, std::string name, FlagSet& value);
    bool implicit_compare_value(FlagSet& value);

    void implicit_compare_check(bool& success);


    template <typename T> void build_json(nlohmann::json& object, std::string name, T& value) {
        if (implicit_compare_value(value)) return;
        object[name] = value;
    }

    template <> void build_json<NodePtr>(nlohmann::json& object, std::string name, NodePtr& value);

    template <> void build_json<NodeList>(nlohmann::json& object, std::string name, NodeList& value);

    template <> void build_json<FlagSet>(nlohmann::json& object, std::string name, FlagSet& value);

    template <typename T, typename ...Ts> void implicit_compare_check(bool& success, T& arg, Ts&... args) {
        if (!implicit_compare_value(arg)) {
            success = false;
            return;
        }
        implicit_compare_check(success,args...);
    }

    template <size_t idx> void compare_helper_check(nlohmann::json& object, bool& success, std::vector<std::string>& names) {
         if (idx != names.size()) success = false;
    }
    template <size_t idx, typename T, typename ...Ts> void compare_helper_check(nlohmann::json& object, bool& success, std::vector<std::string>& names, T& arg, Ts&... args) {
        if (idx >= names.size()) {
            success = false;
            return;
        }
        if (!compare_helper(object,names[idx],arg)) {
            success = false;
            return;
        }
        compare_helper_check<idx+1>(object, success, names, args...);
    }

    template <typename ...T> bool compare_helper(nlohmann::json& object, std::string type, std::vector<std::string> arg_names, T&... args) {
        //Check if everything is implicit if nothing else
        bool success = true;

        if (!object.is_object()) {
            implicit_compare_check(success,args...);
            return success;
        }

        if (!object.contains("type")) return false;
        if (object["type"] != type) return false;
        compare_helper_check<0>(object, success, arg_names, args...);
        return success;
    }

    template <size_t idx> void build_json_helper(nlohmann::json& target, std::vector<std::string> arg_names) {
    }

    template <size_t idx, typename T, typename... Ts> void build_json_helper(nlohmann::json& target, std::vector<std::string> arg_names, T& arg, Ts&... args) {
        if (idx >= arg_names.size()) return;

        build_json(target,arg_names[idx],arg);
        build_json_helper<idx+1>(target,arg_names,args...);
    }

    template <typename ...Ts> nlohmann::json build_json(std::string type, std::vector<std::string> arg_names, Ts&... args) {
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

#define TERMINAL_NODE(N,T) struct N : public Node { \
    N(Coordinate location) : Node(location) {}     \
    void nested_display(std::uint32_t nesting) override {   \
        NOT_IMPL                                   \
    }                                              \
    nlohmann::json as_json() override {             \
        return build_json(T,{});                   \
    }                                                \
    bool compare_json(nlohmann::json o) override {   \
        return compare_helper(o,T,{});               \
    }                                              \
    ~N() override = default;                       \
};


#define SINGLE_MEMBER_NODE(N,T,C,CN) struct N : public Node { \
    C child;                                                 \
    N(Coordinate location, C child) : Node(location), child(child) {} \
    void nested_display(std::uint32_t nesting) override {     \
        NOT_IMPL                                              \
    }                                                         \
    nlohmann::json as_json() override {                       \
        return build_json(T,{CN},child);                       \
    }                                                         \
    bool compare_json(nlohmann::json o) override {            \
        return compare_helper(o,T,{},child);                       \
    }                                                         \
    ~N() override = default;\
};

#define SINGLE_CHILD_NODE(N,T) SINGLE_MEMBER_NODE(N,T,NodePtr,"child")



#endif //CHEESE_NODE_H
