//
// Created by Lexi Allen on 5/27/2023.
//
#include "bacteria/nodes/reciever_nodes.h"
#include "bacteria/BacteriaNode.h"
#include "bacteria/nodes/expression_nodes.h"
#include "NotImplementedException.h"
#include <typeinfo>
#include <utility>
#include "project/GlobalContext.h"
#include "bacteria/BacteriaContext.h"
#include "bacteria/FunctionContext.h"
#include "bacteria/ScopeContext.h"

namespace cheese::bacteria::nodes {
    std::map<std::string, int> BacteriaProgram::get_child_map() const {
        std::map<std::string, int> map;
        for (int i = 0; i < children.size(); i++) {
            auto &child = children[i];
            if (auto as_function = dynamic_cast<Function *>(child.get()); as_function) {
                map[as_function->name] = i;
            } else if (auto as_init = dynamic_cast<VariableInitializationNode *>(child.get()); as_init) {
                map[as_init->name] = i;
            } else if (auto as_proto = dynamic_cast<FunctionImport *>(child.get()); as_proto) {
                map[as_proto->name] = i;
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
                                      const std::vector<BacteriaType *> &childTypes, const std::string &structName,
                                      const bool constant_ref) {
        // Lets deduplicate all types
        for (auto &already_made_type: all_types) {
            if (already_made_type->matches(type, integerSize, subtype, arrayDimensions, childTypes, structName,
                                           constant_ref)) {
                return already_made_type;
            }
        }

        auto ty = new BacteriaType(type, integerSize, subtype, arrayDimensions, childTypes, structName, constant_ref);
        all_types.push_back(ty);
        if (!structName.empty()) {
            named_types[structName] = ty;
        }
        return ty;
    }

    std::unique_ptr<llvm::Module> BacteriaProgram::lower_into_module(project::GlobalContext *ctx) {
        auto bacteriaModule = ctx->gc.gcnew<BacteriaContext>(ctx);
        ctx->gc.add_root_object(bacteriaModule);
        ctx->gc.remove_root_object(ctx);
        // Set up top level declaration info
        for (const auto &child: children) {
            child->gen_protos(bacteriaModule);
        }
        // Lower stuff
        for (const auto &child: children) {
            child->lower_top_level(bacteriaModule);
        }

        return std::unique_ptr<llvm::Module>(bacteriaModule->program_module);
    }

    void Function::lower_top_level(BacteriaContext *ctx) {
        auto prototype = ctx->functions[name].prototype;
        FunctionContext fctx{ctx, prototype};
        fctx.return_type = return_type;
        ScopeContext sctx{fctx, fctx.entry_block};
        size_t idx = 0;
        for (auto &arg: prototype->args()) {
            fctx.all_variables[arguments[idx].name] = VariableInfo{
                    true,
                    arguments[idx].name,
                    arguments[idx].type,
                    &arg
            };
            sctx.variable_renames[arguments[idx].name] = arguments[idx].name;
        }

        for (const auto &child: children) {
            child->lower_scope_level(sctx);
        }
    }

    void Function::gen_protos(BacteriaContext *ctx) {
        auto returnType = return_type->get_llvm_type(ctx->global_context);
        std::vector<llvm::Type *> argTypes;
        std::vector<TypePtr> bacteriaArgTypes;
        for (auto &arg: arguments) {
            argTypes.push_back(arg.type->get_llvm_type(ctx->global_context));
            bacteriaArgTypes.push_back(arg.type);
        }
        auto functionType = llvm::FunctionType::get(returnType, argTypes, false);
        auto prototype = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name,
                                                ctx->program_module);
        size_t idx = 0;
        for (auto &arg: prototype->args()) {
            arg.setName(arguments[idx++].name);
        }
        ctx->functions[name] = FunctionInfo{
                bacteriaArgTypes,
                return_type,
                name,
                prototype
        };
    }
}
