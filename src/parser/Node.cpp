//
// Created by Lexi Allen on 9/21/2022.
//

#include "parser/Node.h"
namespace cheese::parser {

    std::shared_ptr<Node> Node::get() {
        return NodePtr{this};
    }
}