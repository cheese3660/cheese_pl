//
// Created by Lexi Allen on 9/21/2022.
//

#ifndef CHEESE_NODE_H
#define CHEESE_NODE_H

#include <cstdint>
#include <memory>
#include "thirdparty/json.hpp"
#include "Coordinate.h"
namespace cheese::parser {
    class Node {
    public:
        cheese::Coordinate location;
        Node(cheese::Coordinate location) : location(location) {}
        virtual void nested_display(std::uint32_t nesting) = 0;
        virtual nlohmann::json as_json() = 0;
        virtual bool compare_json(nlohmann::json) = 0;
        virtual ~Node() = default;
        std::shared_ptr<Node> get();
    };
    typedef std::shared_ptr<Node> NodePtr;
    typedef std::vector<NodePtr> NodeList;
}


#endif //CHEESE_NODE_H
