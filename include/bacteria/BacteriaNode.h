//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_BACTERIANODE_H
#define CHEESE_BACTERIANODE_H

#include "lexer/lexer.h"
#include "../../external/json.hpp"
#include "math/BigInteger.h"
#include "BacteriaType.h"
#include <memory>
#include <utility>

namespace cheese::bacteria {

    // We are going to completely redefine all the parser helper functions again to make stuff easier on us

    class BacteriaNode {
    public:
        cheese::Coordinate location;

        explicit BacteriaNode(cheese::Coordinate location) : location(location) {}

        virtual ~BacteriaNode() = default;


        std::unique_ptr<BacteriaNode> get();

        inline std::string get_textual_representation() {
            return get_textual_representation(0);
        }

        virtual std::string get_textual_representation(int depth) = 0;

        [[nodiscard]] virtual nlohmann::json as_json() const = 0;

        [[nodiscard]] virtual bool compare_json(const nlohmann::json &json) const = 0;
    };

    void add_indentation(std::stringstream &ss, int indentation);

    typedef std::unique_ptr<BacteriaNode> BacteriaPtr;
    typedef std::vector<BacteriaPtr> BacteriaList;
    typedef std::map<std::string, BacteriaPtr> BacteriaDict;


    bool compare_helper(const nlohmann::json &object, const std::string &name, const BacteriaPtr &value);

    bool implicit_compare_value(const BacteriaPtr &value);


    bool compare_helper(const nlohmann::json &object, const std::string &name, const std::string &value);

    bool implicit_compare_value(const std::string &value);

    bool compare_helper(const nlohmann::json &object, std::string name, bool value);

    bool implicit_compare_value(bool value);

    bool compare_helper(const nlohmann::json &object, const std::string &name, const BacteriaList &value);

    bool implicit_compare_value(const BacteriaList &value);

    bool compare_helper(const nlohmann::json &object, const std::string &name, std::uint64_t value);

    bool implicit_compare_value(std::uint64_t value);

    bool compare_helper(const nlohmann::json &object, const std::string &name, int64_t value);

    bool implicit_compare_value(std::int64_t value);

    bool compare_helper(const nlohmann::json &object, const std::string &name, double value);

    bool implicit_compare_value(double value);

    bool compare_helper(const nlohmann::json &object, const std::string &name, char *value);

    bool compare_helper(const nlohmann::json &object, const std::string &name, const math::BigInteger &value);

    bool implicit_compare_value(const math::BigInteger &value);

    bool compare_helper(const nlohmann::json &object, const std::string &name, const BacteriaDict &value);

    bool implicit_compare_value(const BacteriaDict &value);

    void implicit_compare_check(bool &success);

    template<typename T>
    bool compare_helper(const nlohmann::json &object, const std::string &name, const std::optional<T> &value) {
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
        object[std::move(name)] = value;
    }

    template<>
    void build_json<BacteriaPtr>(nlohmann::json &object, std::string name, const BacteriaPtr &value);

    template<>
    void build_json<BacteriaList>(nlohmann::json &object, std::string name, const BacteriaList &value);

    template<>
    void build_json<BacteriaDict>(nlohmann::json &object, std::string name, const BacteriaDict &value);


    template<>
    void build_json<math::BigInteger>(nlohmann::json &object, std::string name, const math::BigInteger &value);

    template<typename J>
    void build_json(nlohmann::json &object, std::string name, const std::optional<J> &value) {
        if (implicit_compare_value(value)) return;
        return build_json(object, name, value.value());
    }


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
    void
    build_json_helper(nlohmann::json &target, std::vector<std::string> arg_names, const T &arg, const Ts &... args) {
        if (idx >= arg_names.size()) return;

        build_json(target, arg_names[idx], arg);
        build_json_helper<idx + 1>(target, arg_names, args...);
    }

    template<typename ...Ts>
    nlohmann::json build_json(std::string type, std::vector<std::string> arg_names, const Ts &... args) {
        bool all_implicit = true;
        implicit_compare_check(all_implicit, args...);
        if (all_implicit) return type;
        auto result = nlohmann::json::object();
        result["type"] = type;
        build_json_helper<0>(result, arg_names, args...);
        return result;
    }

    struct FunctionArgument {
        TypePtr type;
        std::string name;
    };

    template<>
    void cheese::bacteria::build_json<std::vector<FunctionArgument>>(nlohmann::json &object, std::string name,
                                                                     const std::vector<FunctionArgument> &value);

    bool
    compare_helper(const nlohmann::json &object, const std::string &name,
                   const std::vector<FunctionArgument> &value);

    bool implicit_compare_value(const std::vector<FunctionArgument> &value);
}


#endif //CHEESE_BACTERIANODE_H
