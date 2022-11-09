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
    void parser::build_json<NodePtr>(nlohmann::json &object, std::string name, NodePtr &value) {
        if (value.get() == nullptr) return;
        object[name] = value->as_json();
    }

    template<>
    void parser::build_json<NodeList>(nlohmann::json &object, std::string name, NodeList &value) {
        if (value.empty()) return;
        object[name] = nlohmann::json::array();
        for (auto& n : value) {
            object[name].push_back(n->as_json());
        }
    }

    bool implicit_compare_value(NodePtr &value) {
        return value.get() == nullptr;
    }

    bool parser::compare_helper(nlohmann::json &object, std::string name, std::string &value) {
        if (!object.contains(name)) return value.empty();
        if (!object[name].is_string()) return false;
        return object[name] == value;
    }

    bool implicit_compare_value(std::string &value) {
        return value.empty();
    }

    bool parser::compare_helper(nlohmann::json &object, std::string name, bool &value) {
        if (!object.contains(name)) return !value;
        if (!object[name].is_boolean()) return false;
        return object[name].get<bool>() && value;
    }

    bool implicit_compare_value(bool &value) {
        return !value;
    }

    bool parser::compare_helper(nlohmann::json &object, std::string name, NodeList &value) {
        if (!object.contains(name)) return value.empty();
        if (!object[name].is_array()) return false;
        auto& j_arr = object[name];
        if (j_arr.size() != value.size()) return false;
        for (size_t i = 0; i < value.size(); i++) {
            if (!value[i]->compare_json(j_arr[i])) return false;
        }
        return true;
    }

    bool implicit_compare_value(NodeList &value) {
        return value.empty();
    }

    bool parser::compare_helper(nlohmann::json &object, std::string name, uint64_t &value) {
        if (!object.contains(name)) return value == 0;
        if (!object[name].is_number() || object[name].is_number_float()) return false;
        return object[name] == value;
    }

    bool implicit_compare_value(uint64_t &value) {
        return value == 0;
    }

    bool parser::compare_helper(nlohmann::json &object, std::string name, int64_t &value) {
        if (!object.contains(name)) return value == 0;
        if (!object[name].is_number() || object[name].is_number_float()) return false;
        return object[name] == value;
    }

    bool implicit_compare_value(int64_t &value) {
        return value == 0;
    }

    bool implicit_compare_value(double &value) {
        return value == 0;
    }

    bool parser::compare_helper(nlohmann::json &object, std::string name, const char *value) {
        std::string v = value;
        return compare_helper(object,name,v);
    }

    bool compare_helper(nlohmann::json &object, std::string name, NodePtr &value) {
        if (!object.contains(name)) return false;
        return value->compare_json(object[name]);
    }

    bool compare_helper(nlohmann::json &object, std::string name, double &value) {
        if (!object.contains(name)) return value == 0;
        if (!object[name].is_number()) return false;
        return object[name] == value;
    }

    bool parser::compare_helper(nlohmann::json &object, std::string name, FlagSet &value) {
        if (!object.contains(name)) return implicit_compare_value(value);
        auto list = object[name];
        bool inlin = value.inlin;
        bool exter = value.exter;
        bool exp = value.exp;
        bool comptime = value.comptime;
        bool pub = value.pub;
        bool priv = value.priv;
        bool cons = value.cons;
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
            } else if (str == "const") {
                cons = !cons;
            }
        }
        return !(inlin || exter || exp || comptime || pub || priv || cons);
    }

    bool parser::implicit_compare_value(FlagSet &value) {
        return !(value.inlin || value.exter || value.exp || value.comptime || value.pub || value.priv || value.cons);
    }

    template<>
    void parser::build_json<FlagSet>(nlohmann::json &object, std::string name, FlagSet &value) {
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
        if (value.cons) {
            object[name].push_back("const");
        }
    }

}