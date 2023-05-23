//
// Created by Lexi Allen on 4/2/2023.
//

#ifndef CHEESE_RECIEVER_NODES_H
#define CHEESE_RECIEVER_NODES_H

#include "bacteria/BacteriaReciever.h"
#include "bacteria/BacteriaType.h"
#include <sstream>

namespace cheese::bacteria::nodes {

    struct BacteriaProgram : BacteriaReciever {

        BacteriaProgram(Coordinate location) : BacteriaReciever(location) {

        }

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            for (auto &child: children) {
                add_indentation(ss, depth);
                ss << child->get_textual_representation(depth);
                ss << "\n";
            }
            return ss.str();
        }

        ~BacteriaProgram() override = default;
    };

    struct UnnamedBlock : BacteriaReciever {
        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "{\n";
            for (auto &child: children) {
                add_indentation(ss, depth + 1);
                ss << child->get_textual_representation(depth + 1);
                ss << "\n";
            }
            add_indentation(ss, depth);
            ss << "}";
            return ss.str();
        }
    };

    struct FunctionArgument {
        bacteria::TypePtr type;
        std::string name;
    };

    struct Function : BacteriaReciever {
        Function(Coordinate location, std::string n, std::vector<FunctionArgument> args, bacteria::TypePtr rt)
                : BacteriaReciever(location), name(n), arguments(args), return_type(rt) {

        }

        std::string name;
        std::vector<FunctionArgument> arguments;
        bacteria::TypePtr return_type;

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            ss << "fn " << name << " ";
            for (int i = 0; i < arguments.size(); i++) {
                auto &argument = arguments[i];
                ss << argument.name << ": " << argument.type->to_string();
                if (i < arguments.size() - 1) {
                    ss << ", ";
                } else {
                    ss << " ";
                }
            }
            ss << "=> " << return_type->to_string();
            ss << " {\n";
            for (auto &child: children) {
                add_indentation(ss, depth + 1);
                ss << child->get_textual_representation(depth + 1);
                ss << "\n";
            }
            ss << "}";
            return ss.str();
        }
    };
}

#endif //CHEESE_RECIEVER_NODES_H
