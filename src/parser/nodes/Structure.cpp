//
// Created by Lexi Allen on 9/21/2022.
//
#include "parser/nodes/Structure.h"
#include "NotImplementedException.h"
namespace cheese::parser::nodes{
    void Structure::nested_display(std::uint32_t nesting) {
        NOT_IMPL
    }

    nlohmann::json Structure::as_json() {
        NOT_IMPL
    }

    bool Structure::compare_json(nlohmann::json json) {
        if (!json.is_object()) return false;
        if (!json.contains("type")) return false;
        if (!json["type"].is_string()) return false;
        if (json["type"].get<std::string>() != "structure") return false;
        if (!json.contains("tuple")) return false;
        if (!json["tuple"].is_boolean()) return false;
        if (json["tuple"].get<bool>() != is_tuple) return false;
        if (!json.contains("children")) return false;
        if (!json["children"].is_array()) return false;
        if (json["children"].size() != children.size()) return false;
        auto& j_children = json["children"];
        for (size_t i = 0; i < children.size(); i++) {
            if (!children[i]->compare_json(j_children[i])) return false;
        }
        return true;
    }
}