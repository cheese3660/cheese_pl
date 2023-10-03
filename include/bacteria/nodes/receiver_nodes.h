//
// Created by Lexi Allen on 4/2/2023.
//

#ifndef CHEESE_RECEIVER_NODES_H
#define CHEESE_RECEIVER_NODES_H

#include "bacteria/BacteriaReceiver.h"
#include "bacteria/BacteriaType.h"
#include "parser/Node.h"
#include <sstream>
#include <utility>
#include <llvm/IR/Module.h>

namespace cheese::bacteria::nodes {

    struct BacteriaProgram : public BacteriaReceiver {

        BacteriaProgram(Coordinate location);

        TypeList all_types = {};
        TypeDict named_types = {};

        std::string get_textual_representation(int depth) override {
            std::stringstream ss{};
            for (auto &type: named_types) {
                add_indentation(ss, depth);
                ss << type.first << ": type = " << type.second->to_string(true) << '\n';
            }
            for (auto &child: children) {
                add_indentation(ss, depth);
                ss << child->get_textual_representation(depth);
                ss << "\n";
            }
            return ss.str();
        }

        TypePtr get_type(BacteriaType::Type type = BacteriaType::Type::Void, uint16_t integerSize = 0,
                         BacteriaType *subtype = {},
                         const std::vector<std::size_t> &arrayDimensions = {},
                         const std::vector<BacteriaType *> &childTypes = {},
                         const std::string &structName = {},
                         const bool constant_ref = {});


        ~BacteriaProgram() override {
            for (auto type: all_types) {
                delete type;
            }
        }


        [[nodiscard]] std::map<std::string, int> get_child_map() const;


        // For this we actually want to return a dictionary with top level declarations, rather than anything else as it will make generating tests simpler as ordering is wierd
        [[nodiscard]] nlohmann::json as_json() const override {
            auto object = nlohmann::json::object();
            for (auto &kv: named_types) {
                object[kv.first] = kv.second->to_string(true);
            }
            auto map = get_child_map();
            for (auto &kv: map) {
                object[kv.first] = children[kv.second]->as_json();
            }
            return object;
        }

        [[nodiscard]] bool compare_json(const nlohmann::json &json) const override {
            // This is going to be interesting
            if (!json.is_object()) return false;
            if (json.size() != children.size() + named_types.size()) return false;
            auto map = get_child_map();
            for (auto &kv: map) {
                if (!json.contains(kv.first)) continue;
                if (!children[kv.second]->compare_json(json[kv.first])) return false;
            }
            return std::ranges::all_of(named_types.cbegin(), named_types.cend(),
                                       [&](const std::pair<std::string, TypePtr> &kv) {
                                           return !json.contains(kv.first) || kv.second->to_string(true) ==
                                                                              json[kv.first].get<std::string>();
                                       });
        }

        std::unique_ptr<llvm::Module> lower_into_module(project::GlobalContext *ctx);
    };

    struct UnnamedBlock : BacteriaReceiver {
        UnnamedBlock(Coordinate location) : BacteriaReceiver(location) {}

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

        JSON_FUNCS("block", { "body" }, children)

        ~UnnamedBlock() override = default;

        void lower_scope_level(ScopeContext &ctx) override;

        llvm::Value *lower_expression_level(ScopeContext &ctx, ExpressionContext &expr) override;;
    };


    struct Function : BacteriaReceiver {
        Function(Coordinate location, std::string n, std::vector<FunctionArgument> args, bacteria::TypePtr rt)
                : BacteriaReceiver(location), name(std::move(n)), arguments(args), return_type(rt) {

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

        void gen_protos(BacteriaContext *ctx) override;

        void lower_top_level(BacteriaContext *ctx) override;
    };
}

#endif //CHEESE_RECEIVER_NODES_H
