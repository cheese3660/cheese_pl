//
// Created by Lexi Allen on 5/27/2023.
//
#include "bacteria/nodes/reciever_nodes.h"
#include "bacteria/BacteriaNode.h"
#include "NotImplementedException.h"
#include <typeinfo>

namespace cheese::bacteria::nodes {
    std::map<std::string, int> BacteriaProgram::get_child_map() const {
        std::map<std::string, int> map;
        for (int i = 0; i < children.size(); i++) {
            auto &child = children[i];
            if (auto as_function = dynamic_cast<Function *>(child.get()); as_function) {
                map[as_function->name] = i;
            } else {
                auto c = child.get();
                NOT_IMPL_FOR(typeid(*c).name());
            }
        }
        return map;
    }
}
