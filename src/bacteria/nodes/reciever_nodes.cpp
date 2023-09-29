//
// Created by Lexi Allen on 5/27/2023.
//
#include "bacteria/nodes/reciever_nodes.h"
#include "bacteria/BacteriaNode.h"
#include "NotImplementedException.h"
#include <typeinfo>
#include <utility>

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


    BacteriaProgram::BacteriaProgram(Coordinate location) : BacteriaReceiver(location) {

    }

    TypePtr BacteriaProgram::get_type(BacteriaType::Type type, uint16_t integerSize, BacteriaType *subtype,
                                      const std::vector<std::size_t> &arrayDimensions,
                                      const std::vector<BacteriaType *> &childTypes, const std::string &structName) {
        // Lets deduplicate all types
        for (auto &already_made_type: all_types) {
            if (already_made_type->matches(type, integerSize, subtype, arrayDimensions, childTypes, structName)) {
                return already_made_type;
            }
        }

        auto ty = new BacteriaType(type, integerSize, subtype, arrayDimensions, childTypes, structName);
        all_types.push_back(ty);
        if (!structName.empty()) {
            named_types[structName] = ty;
        }
        return ty;
    }
}
