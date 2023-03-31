//
// Created by Lexi Allen on 3/20/2023.
//

#include "bacteria/BacteriaNode.h"

namespace cheese::bacteria {

    std::unique_ptr<BacteriaNode> BacteriaNode::get() {
        return std::unique_ptr<BacteriaNode>(this);
    }
}