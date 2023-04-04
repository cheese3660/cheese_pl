//
// Created by Lexi Allen on 11/7/2022.
//

#include "parser/nodes/other_nodes.h"
#include "NotImplementedException.h"

namespace cheese::parser::nodes {

    nlohmann::json Import::as_json() const {
//        auto result = nlohmann::json::object();
//        result["type"] = "import";
//        result["path"] = path;
//        result["name"] = name;
        return build_json("import", {"path", "name"}, path, name);
    }

    bool Import::compare_json(const nlohmann::json &json) const {
        return compare_helper(json, "import", {
                "path",
                "name"
        }, path, name);
    }
}