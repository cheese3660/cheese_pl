//
// Created by Lexi Allen on 9/21/2022.
//

#ifndef CHEESE_STRUCTURE_H
#define CHEESE_STRUCTURE_H
#include "../Node.h"
#include <vector>
#include <memory>

namespace cheese::parser::nodes {
    struct Structure : public Node {
        bool is_tuple;
        NodeList interfaces;
        NodeList children;
        Structure(Coordinate location, NodeList interfaces, NodeList children, bool is_tuple) : Node(location), interfaces(std::move(interfaces)), children(std::move(children)), is_tuple(is_tuple) {}
        void nested_display(std::uint32_t nesting) override;
        nlohmann::json as_json() override;
        bool compare_json(nlohmann::json) override;
        ~Structure() override = default;
    };
}


#endif //CHEESE_STRUCTURE_H
