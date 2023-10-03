//
// Created by Lexi Allen on 9/28/2023.
//
#include "bacteria/BacteriaReceiver.h"
#include "bacteria/nodes/receiver_nodes.h"
#include "bacteria/nodes/expression_nodes.h"

namespace cheese::bacteria {

    void BacteriaReceiver::receive(BacteriaPtr node) {
        children.push_back(std::move(node));
    }

}