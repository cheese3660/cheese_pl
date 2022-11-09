//
// Created by Lexi Allen on 11/7/2022.
//

#include "parser/nodes/Import.h"
#include "NotImplementedException.h"
namespace cheese::parser::nodes {
    void Import::nested_display(std::uint32_t nesting) {
        NOT_IMPL
    }

    nlohmann::json Import::as_json() {
//        auto result = nlohmann::json::object();
//        result["type"] = "import";
//        result["path"] = path;
//        result["name"] = name;
        return build_json("import",{"path","name"},path,name);
    }

    bool Import::compare_json(nlohmann::json json) {
        return compare_helper(json,"import",{
            "path",
            "name"
        }, path, name);
    }
}