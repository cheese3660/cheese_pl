//
// Created by Lexi Allen on 4/2/2023.
//

#ifndef CHEESE_BACTERIARECEIVER_H
#define CHEESE_BACTERIARECEIVER_H

#include <utility>

#include "BacteriaNode.h"
#include "BacteriaType.h"

namespace cheese::bacteria {

    struct BacteriaReceiver : BacteriaNode {
        BacteriaList children;

        BacteriaReceiver(Coordinate location) : BacteriaNode(location) {

        }


        void receive(BacteriaPtr node);

        ~BacteriaReceiver() override = default;

    private:
    };

    typedef std::unique_ptr<BacteriaReceiver> Reciever;
}

#endif //CHEESE_BACTERIARECEIVER_H
