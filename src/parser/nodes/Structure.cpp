//
// Created by Lexi Allen on 9/21/2022.
//
#include "parser/nodes/other_nodes.h"
#include "NotImplementedException.h"
#include "curdle/Structure.h"

namespace cheese::parser::nodes {

    nlohmann::json Structure::as_json() const {
//        if (!is_tuple && interfaces.size() == 0 && children.size() == 0) {
//            return "struct";
//        }
//        nlohmann::json ints = nlohmann::json::array();
//        nlohmann::json kids = nlohmann::json::array();
//        for (auto &interface: interfaces) {
//            ints.push_back(interface->as_json());
//        }
//        for (auto &child: children) {
//            kids.push_back(child->as_json());
//        }
//        nlohmann::json result = nlohmann::json::object();
//        result["type"] = "struct";
//        if (is_tuple) result["tuple"] = true;
//        if (ints.size() > 0) result["interfaces"] = ints;
//        if (kids.size() > 0) result["children"] = kids;
//        return result;
        return build_json("struct", {"tuple", "interfaces", "children"}, is_tuple, interfaces, children);
    }

    bool Structure::compare_json(const nlohmann::json &json) const {
        //Basically if it is as simple as possible and can just be reduced to struct then do so
//        if (json.is_string() && json.get<std::string>() == "struct" && !is_tuple && interfaces.size() == 0 && children.size() == 0) return true;
//        if (!json.is_object()) return false;
//        if (!compare_helper(json,"type","struct")) return false;
//        if (!compare_helper(json,"tuple",is_tuple)) return false;
//        if (!compare_helper(json, "interfaces",interfaces)) return false;
//        if (!compare_helper(json, "children", children)) return false;
        return compare_helper(json, "struct", {
                "tuple",
                "interfaces",
                "children"
        }, is_tuple, interfaces, children);


    }
}

