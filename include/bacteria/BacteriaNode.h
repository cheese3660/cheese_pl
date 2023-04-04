//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_BACTERIANODE_H
#define CHEESE_BACTERIANODE_H

#include "lexer/lexer.h"
#include <memory>

namespace cheese::bacteria {


    class BacteriaNode {
    public:
        cheese::Coordinate location;

        explicit BacteriaNode(cheese::Coordinate location) : location(location) {}

        virtual ~BacteriaNode() = default;


        std::unique_ptr<BacteriaNode> get();

        inline std::string get_textual_representation() {
            return get_textual_representation(0);
        }

        virtual std::string get_textual_representation(int depth) = 0;
    };

    void add_indentation(std::stringstream &ss, int indentation);

    typedef std::unique_ptr<BacteriaNode> BacteriaPtr;
    typedef std::vector<BacteriaPtr> BacteriaList;
    typedef std::map<std::string, BacteriaPtr> BacteriaDict;
}


#endif //CHEESE_BACTERIANODE_H
