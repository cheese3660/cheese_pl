//
// Created by Lexi Allen on 3/20/2023.
//

#include "bacteria/BacteriaNode.h"
#include <sstream>

namespace cheese::bacteria {

    std::unique_ptr<BacteriaNode> BacteriaNode::get() {
        return std::unique_ptr<BacteriaNode>(this);
    }

    void add_indentation(std::stringstream &ss, int indentation) {
        for (int i = 0; i < indentation; i++) {
            ss << "    ";
        }
    }

    bool bacteria::implicit_compare_value(const std::string &value) {
        return value.empty();
    }

    bool bacteria::implicit_compare_value(bool value) {
        return value == true;
    }

    bool bacteria::implicit_compare_value(const BacteriaList &value) {
        return value.empty();
    }

    bool bacteria::implicit_compare_value(std::uint64_t value) {
        return value == 0;
    }

    bool bacteria::implicit_compare_value(std::int64_t value) {
        return value == 0;
    }

    bool bacteria::implicit_compare_value(double value) {
        return value == 0;
    }

    bool bacteria::implicit_compare_value(const math::BigInteger &value) {
        return value == 0;
    }

    bool bacteria::implicit_compare_value(const BacteriaDict &value) {
        return value.empty();
    }

#define CATCH_IMPLICIT() if (!object.contains(name)) return implicit_compare_value(value)
#define CATCH_TYPE(type) if (!object[name].is_##type()) return false
#define COMPARE_DIRECT() return object[name] == value

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, const BacteriaPtr &value) {
        CATCH_IMPLICIT();
        return value->compare_json(object[name]);
    }

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, const std::string &value) {
        CATCH_IMPLICIT();
        CATCH_TYPE(string);
        // We always catch an ignore string for temporary names and such
        if (object[name] == "<ignore>") return true;
        COMPARE_DIRECT();
    }

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, std::uint64_t value) {
        CATCH_IMPLICIT();
        CATCH_TYPE(number_integer);
        COMPARE_DIRECT();
    }

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, const BacteriaList &value) {
        CATCH_IMPLICIT();
        CATCH_TYPE(array);
        auto &j_arr = object[name];
        if (j_arr.size() != value.size()) return false;
        for (size_t i = 0; i < value.size(); i++) {
            if (!value[i]->compare_json(j_arr[i])) return false;
        }
        return true;
    }

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, int64_t value) {
        CATCH_IMPLICIT();
        CATCH_TYPE(number_integer);
        COMPARE_DIRECT();
    }

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, double value) {
        CATCH_IMPLICIT();
        CATCH_TYPE(number);
        COMPARE_DIRECT();
    }

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, char *value) {
        return compare_helper(object, name, std::string(value));
    }

    bool
    bacteria::compare_helper(const nlohmann::json &object, const std::string &name, const math::BigInteger &value) {
        CATCH_IMPLICIT();
        const auto &x = object[name];
        if (x.is_number_integer()) {
            return value == x.get<std::int64_t>();
        } else if (x.is_string()) {
            return value == x.get<std::string>();
        }
        return false;
    }

    bool bacteria::compare_helper(const nlohmann::json &object, const std::string &name, const BacteriaDict &value) {
        CATCH_IMPLICIT();
        CATCH_TYPE(object);
        const auto &lst = object[name];
        if (lst.size() != value.size()) return false;
        for (const auto &kv: value) {
            if (!lst.contains(kv.first)) return false;
            if (!compare_helper(lst, kv.first, kv.second)) return false;
        }
        return true;
    }

    void bacteria::implicit_compare_check(bool &success) {

    }

    template<>
    void bacteria::build_json<BacteriaList>(nlohmann::json &object, std::string name, const BacteriaList &value) {
        if (value.empty()) return;
        object[name] = nlohmann::json::array();
        for (auto &n: value) {
            object[name].push_back(n->as_json());
        }
    }

    template<>
    void
    bacteria::build_json<math::BigInteger>(nlohmann::json &object, std::string name, const math::BigInteger &value) {
        if (implicit_compare_value(value)) return;
        if (value.words.size() > 1) {
            object[name] = static_cast<std::string>(value);
        } else {
            object[name] = static_cast<std::int64_t>(value);
        }
    }

    template<>
    void bacteria::build_json<BacteriaDict>(nlohmann::json &object, std::string name, const BacteriaDict &value) {
        if (implicit_compare_value(value)) return;
        auto lst = nlohmann::json::object();
        for (const auto &kv: value) {
            build_json(lst, kv.first, kv.second);
        }
        object[name] = lst;
    }

    template<>
    void bacteria::build_json<BacteriaPtr>(nlohmann::json &object, std::string name, const BacteriaPtr &value) {
        if (!implicit_compare_value(value)) object[name] = value->as_json();
    }


    bool compare_helper(const nlohmann::json &object, const std::string &name,
                        const std::vector<FunctionArgument> &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        if (!object[name].is_array()) return false;
        if (object[name].size() != value.size()) return false;
        auto &arr = object[name];
        for (int i = 0; i < value.size(); i++) {
            if (!arr[i].is_object()) return false;
            auto compare = cheese::bacteria::compare_helper(arr[i], "name", value[i].name) &&
                           cheese::bacteria::compare_helper(arr[i], "type", value[i].type->to_string());
            if (!compare) return false;
        }
        return true;
    }

    bool implicit_compare_value(const std::vector<FunctionArgument> &value) {
        return value.empty();
    }

    bool bacteria::implicit_compare_value(const BacteriaPtr &value) {
        return false;
    }


    template<>
    void cheese::bacteria::build_json<std::vector<FunctionArgument>>(nlohmann::json &object, std::string name,
                                                                     const std::vector<FunctionArgument> &value) {
        auto arr = nlohmann::json::array();
        for (auto &v: value) {
            auto obj = nlohmann::json::object();
            build_json(obj, "name", v.name);
            build_json(obj, "type", v.type->to_string());
            arr.push_back(obj);
        }
        object[name] = arr;
    }

#undef CATCH_IMPLICIT
#undef CATCH_TYPE
#undef COMPARE_DIRECT
}