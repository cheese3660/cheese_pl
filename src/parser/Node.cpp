//
// Created by Lexi Allen on 9/21/2022.
//

#include "parser/Node.h"
namespace cheese::parser {

    std::shared_ptr<Node> Node::get() {
        return NodePtr{this};
    }

    void implicit_compare_check(bool &success) {
    }

    template<>
    void parser::build_json<NodePtr>(nlohmann::json &object, std::string name, const NodePtr &value) {
        if (value.get() == nullptr) return;
        object[name] = value->as_json();
    }

    template<>
    void parser::build_json<NodeList>(nlohmann::json &object, std::string name, const NodeList &value) {
        if (value.empty()) return;
        object[name] = nlohmann::json::array();
        for (auto& n : value) {
            object[name].push_back(n->as_json());
        }
    }

    bool implicit_compare_value(const NodePtr &value) {
        return value.get() == nullptr;
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, const std::string &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        if (!object[name].is_string()) return false;
        return object[name] == value;
    }

    bool implicit_compare_value(const std::string &value) {
        return value.empty();
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, bool value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        if (!object[name].is_boolean()) return false;
        return object[name].get<bool>() && value;
    }

    bool implicit_compare_value(bool value) {
        return !value;
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, const NodeList &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        if (!object[name].is_array()) return false;
        auto& j_arr = object[name];
        if (j_arr.size() != value.size()) return false;
        for (size_t i = 0; i < value.size(); i++) {
            if (!value[i]->compare_json(j_arr[i])) return false;
        }
        return true;
    }

    bool implicit_compare_value(const NodeList &value) {
        return value.empty();
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, uint64_t value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        if (!object[name].is_number() || object[name].is_number_float()) return false;
        return object[name] == value;
    }

    bool implicit_compare_value(uint64_t value) {
        return value == 0;
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, int64_t value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        if (!object[name].is_number() || object[name].is_number_float()) return false;
        return object[name] == value;
    }

    bool implicit_compare_value(int64_t value) {
        return value == 0;
    }

    bool implicit_compare_value(double value) {
        return value == 0;
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, char *value) {
        std::string v = value;
        return compare_helper(object,name,v);
    }

    bool compare_helper(const nlohmann::json &object, std::string name, const NodePtr &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        return value->compare_json(object[name]);
    }

    bool compare_helper(const nlohmann::json &object, std::string name, double value) {
        if (!object.contains(name)) return value == 0;
        if (!object[name].is_number()) return false;
        return object[name] == value;
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, const FlagSet &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        auto list = object[name];
        bool inlin = value.inlin;
        bool exter = value.exter;
        bool exp = value.exp;
        bool comptime = value.comptime;
        bool pub = value.pub;
        bool priv = value.priv;
        bool mut = value.mut;
        bool entry = value.entry;
        for (size_t i = 0; i < list.size(); i++) {
            auto& str = list[i];
            if (str == "inline") {
                inlin = !inlin;
            } else if (str == "extern") {
                exter = !exter;
            } else if (str == "export") {
                exp = !exp;
            } else if (str == "comptime") {
                comptime = !comptime;
            } else if (str == "public") {
                pub = !pub;
            } else if (str == "private") {
                priv = !priv;
            } else if (str == "mutable") {
                mut = !mut;
            } else if (str == "entry") {
                entry = !entry;
            }
        }
        return !(inlin || exter || exp || comptime || pub || priv || mut || entry);
    }

    bool parser::implicit_compare_value(const FlagSet &value) {
        return !(value.inlin || value.exter || value.exp || value.comptime || value.pub || value.priv || value.mut || value.entry);
    }

    template<>
    void parser::build_json<FlagSet>(nlohmann::json &object, std::string name, const FlagSet &value) {
        if (implicit_compare_value(value)) return;
        object[name] = nlohmann::json::array();
        if (value.inlin) {
            object[name].push_back("inline");
        }
        if (value.exter) {
            object[name].push_back("extern");
        }
        if (value.exp) {
            object[name].push_back("export");
        }
        if (value.comptime) {
            object[name].push_back("comptime");
        }
        if (value.pub) {
            object[name].push_back("public");
        }
        if (value.priv) {
            object[name].push_back("private");
        }
        if (value.mut) {
            object[name].push_back("mutable");
        }
        if (value.entry) {
            object[name].push_back("entry");
        }
    }

    template<>
    void parser::build_json<math::BigInteger>(nlohmann::json &object, std::string name, const math::BigInteger &value) {
        if (implicit_compare_value(value)) return;
        if (value.words.size() > 1) {
            object[name] = static_cast<std::string>(value);
        } else {
            object[name] = static_cast<std::int64_t>(value);
        }
    }

    bool parser::implicit_compare_value(const math::BigInteger &value) {
        return value.zero();
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, const math::BigInteger &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        auto x = object[name];
        if (x.is_number_integer()) {
            return value == x.get<std::int64_t>();
        } else if (x.is_string()) {
            return value == x.get<std::string>();
        }
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string type) {
        if (!object.is_object()) {
            return object == type;
        } else if (!object.contains("type")) {
            return false;
        } else {
            return object["type"] == type;
        }
    }

    bool parser::compare_helper(const nlohmann::json &object, std::string name, const NodeDict &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        if (!object[name].is_object()) return false;
        const auto& lst = object[name];
        if (lst.size() != value.size()) return false;
        for (const auto& kv : value) {
            if (!lst.contains(kv.first)) return false;
            if (!compare_helper(lst,kv.first,kv.second)) return false;
        }
        return true;
    }

    bool parser::implicit_compare_value(const NodeDict &value) {
        return value.size() == 0;
    }

    template<>
    void parser::build_json<NodeDict>(nlohmann::json &object, std::string name, const NodeDict &value) {
        if (implicit_compare_value(value)) return;
        auto lst = nlohmann::json::object();
        for (const auto& kv : value) {
            build_json(lst,kv.first,kv.second);
        }
        object[name]=lst;
    }


}