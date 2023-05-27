//
// Created by Lexi Allen on 4/2/2023.
//

#ifndef CHEESE_RECIEVER_NODES_H
#define CHEESE_RECIEVER_NODES_H

#include "bacteria/BacteriaReciever.h"
#include "bacteria/BacteriaType.h"
#include "parser/Node.h"
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

        [[nodiscard]] std::map<std::string, int> get_child_map() const;

        // For this we actually want to return a dictionary with top level declarations, rather than anything else as it will make generating tests simpler as ordering is wierd
        [[nodiscard]] nlohmann::json as_json() const override {
            auto object = nlohmann::json::object();
            auto map = get_child_map();
            for (auto &kv: map) {
                object[kv.first] = children[kv.second]->as_json();
            }
            return object;
        }

        [[nodiscard]] bool compare_json(const nlohmann::json &json) const override {
            // This is going to be interesting
            if (!json.is_object()) return false;
            if (json.size() != children.size()) return false;
            auto map = get_child_map();
            for (auto &kv: map) {
                if (!json.contains(kv.first)) return false;
                if (!children[kv.second]->compare_json(json[kv.first])) return false;
            }
            return true;
        }
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

        JSON_FUNCS("block", { "body" }, children);
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

        ~Function() override = default;

        JSON_FUNCS("function", { "name", "arguments", "return_type", "body" }, name, arguments,
                   return_type->to_string(), children);
    };
}

#endif //CHEESE_RECIEVER_NODES_H
