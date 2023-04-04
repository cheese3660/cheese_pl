//
// Created by Lexi Allen on 3/20/2023.
//

#include "bacteria/BacteriaNode.h"
#include <sstream>

namespace cheese::bacteria {

    std::unique_ptr<BacteriaNode> BacteriaNode::get() {
        return std::unique_ptr<BacteriaNode>(this);
    }

    void add_indentation(std::stringstream &ss, int indentation) {
        for (int i = 0; i < indentation; i++) {
            ss << "    ";
        }
    }
}