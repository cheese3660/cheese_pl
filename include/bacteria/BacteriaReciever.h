//
// Created by Lexi Allen on 4/2/2023.
//

#ifndef CHEESE_BACTERIARECIEVER_H
#define CHEESE_BACTERIARECIEVER_H

#include "BacteriaNode.h"

namespace cheese::bacteria {
    struct BacteriaReciever : BacteriaNode {
        BacteriaList children;

        BacteriaReciever(Coordinate location) : BacteriaNode(location) {

        }


        inline void recieve(BacteriaPtr node) {
            children.push_back(std::move(node));
        }

        ~BacteriaReciever() override = default;
    };

    typedef std::unique_ptr<BacteriaReciever> Reciever;
}

#endif //CHEESE_BACTERIARECIEVER_H
