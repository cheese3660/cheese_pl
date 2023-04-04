//
// Created by Lexi Allen on 4/2/2023.
//

#ifndef CHEESE_RECIEVER_NODES_H
#define CHEESE_RECIEVER_NODES_H

#include "bacteria/BacteriaReciever.h"
#include <sstream>

namespace cheese::bacteria::nodes {

    struct BacteriaProgram : BacteriaReciever {

        BacteriaProgram(Coordinate location) : BacteriaReciever(location) {

        }

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "{\n";
            for (auto &child: children) {
                add_indentation(ss, depth);
                ss << child->get_textual_representation(depth);
                ss << "\n";
            }
            ss << "}";
            return ss.str();
        }

        ~BacteriaProgram() override = default;
    };

    struct UnnamedBlock : BacteriaReciever {
        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "{\n";
            for (auto &child: children) {
                add_indentation(ss, depth);
                ss << "\n";
            }
            ss << "}";
            return ss.str();
        }
    };
}

#endif //CHEESE_RECIEVER_NODES_H
