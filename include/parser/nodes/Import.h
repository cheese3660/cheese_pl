//
// Created by Lexi Allen on 11/7/2022.
//

#ifndef CHEESE_IMPORT_H
#define CHEESE_IMPORT_H
#include "../Node.h"
#include <vector>
#include <memory>

namespace cheese::parser::nodes {
    struct Import : public Node {
        std::string path;
        std::string name;

        Import(Coordinate location, std::string path, std::string name) : Node(location), path(path), name(name) {}
        void nested_display(std::uint32_t nesting) override;
        nlohmann::json as_json() override;
        bool compare_json(nlohmann::json) override;
        ~Import() override = default;
    };
}


#endif //CHEESE_IMPORT_H
