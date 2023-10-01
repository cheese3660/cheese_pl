//
// Created by Lexi Allen on 9/30/2023.
//
#include <utility>

#include "curdle/types/ComposedFunctionType.h"
#include "curdle/names.h"
#include "curdle/types/AnyType.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/types/IntegerType.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/curdle.h"
#include "curdle/types/FunctionPointerType.h"
#include "curdle/types/ReferenceType.h"
#include "bacteria/nodes/reciever_nodes.h"
#include "bacteria/nodes/expression_nodes.h"

namespace cheese::curdle {
    std::string get_first(std::vector<Type *> operand_types) {
        return operand_types[0]->to_string();
    }

    std::string get_second(std::vector<Type *> operand_types) {
        return operand_types[1]->to_string();
    }

    std::string get_name(enums::SimpleOperation operation, const std::vector<Type *> &operand_types) {
        std::stringstream name;
        name << "::(";
        auto op_string = enums::get_op_string(operation);
        switch (operation) {
            case enums::SimpleOperation::UnaryPlus:
            case enums::SimpleOperation::UnaryMinus:
            case enums::SimpleOperation::Not:
                name << op_string << get_first(operand_types);
                break;
            case enums::SimpleOperation::Multiplication:
            case enums::SimpleOperation::Division:
            case enums::SimpleOperation::Remainder:
            case enums::SimpleOperation::Addition:
            case enums::SimpleOperation::Subtraction:
            case enums::SimpleOperation::LeftShift:
            case enums::SimpleOperation::RightShift:
            case enums::SimpleOperation::LesserThan:
            case enums::SimpleOperation::GreaterThan:
            case enums::SimpleOperation::LesserThanOrEqualTo:
            case enums::SimpleOperation::GreaterThanOrEqualTo:
            case enums::SimpleOperation::EqualTo:
            case enums::SimpleOperation::NotEqualTo:
            case enums::SimpleOperation::And:
            case enums::SimpleOperation::Or:
            case enums::SimpleOperation::Xor:
            case enums::SimpleOperation::Combine:
            case enums::SimpleOperation::Compose:
                name << get_first(operand_types) << op_string
                     << get_second(operand_types);
                break;
        }
        name << ")";
        return name.str();
    }

    ComposedFunctionType *
    ComposedFunctionType::get(GlobalContext *gctx, enums::SimpleOperation operation,
                              const std::vector<Type *> &operand_types) {
        auto cached_name = "type: " + get_name(operation, operand_types);
        if (!gctx->cached_objects.contains(cached_name)) {
            auto ref = gctx->gc.gcnew<ComposedFunctionType>(operation, operand_types);
            gctx->cached_objects[cached_name] = ref;
            return ref;
        }
        return dynamic_cast<ComposedFunctionType *>(gctx->cached_objects[cached_name]);
    }

    bacteria::TypePtr ComposedFunctionType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        auto base_type = program->get_type(bacteria::BacteriaType::Type::Object, 0, {}, {}, {},
                                           mangle(get_name(operation, operand_types)));
        for (const auto &rtime: operand_types) {
            base_type->child_types.push_back(rtime->get_cached_type(program));
        }
        return base_type;
    }

    void ComposedFunctionType::mark_type_references() {
        for (const auto &rtime: operand_types) {
            if (rtime != nullptr) rtime->mark();
        }
    }

    Comptimeness ComposedFunctionType::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t ComposedFunctionType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        return -1;
    }

    gcref<Type> ComposedFunctionType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        PEER_TYPE_CATCH_ANY();
        if (other == this) REF(this);
        NO_PEER;
    }

    gcref<ComptimeValue>
    ComposedFunctionType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        INVALID_CHILD;
    }

    std::string ComposedFunctionType::to_string() {
        return get_name(operation, operand_types);
    }

    ComposedFunctionType::ComposedFunctionType(enums::SimpleOperation operation,
                                               std::vector<Type *> operand_types) : operation(operation),
                                                                                    operand_types(
                                                                                            std::move(
                                                                                                    operand_types)) {

    }


    gcref<Type> ComposedFunctionType::get_return_type(cheese::project::GlobalContext *gctx) {
        Type *first_ty = operand_types[0];
        if (is_functional_type(first_ty)) first_ty = get_functional_return_type(first_ty, gctx);
        if (!enums::is_binary_op(operation)) return unary_result_type(operation, first_ty, gctx);
        auto second_ty = operand_types[1];
        if (is_functional_type(second_ty)) second_ty = get_functional_return_type(second_ty, gctx);
        return binary_result_type(operation, first_ty, second_ty, gctx);
    }

    std::vector<gcref<Type>> ComposedFunctionType::get_argument_types(cheese::project::GlobalContext *gctx) {
        for (const auto &operand: operand_types) {
            if (is_functional_type(operand)) {
                return get_functional_argument_types(operand, gctx);
            }
        }
        return {};
    }

    std::string ComposedFunctionType::get_function_name(cheese::project::ComptimeContext *cctx) {
        if (!cached_function_name.empty()) return cached_function_name;
        cached_function_name = mangle(to_string() + ":fn");
        // Lets start building up the AST for the function, and the context as well
        auto gctx = cctx->globalContext;
        auto rctx = gctx->gc.gcnew<RuntimeContext>(cctx, cctx->currentStructure);
        auto arg_types = get_argument_types(gctx);
        auto ref = gctx->gc.gcnew<ReferenceType>(this, true);
        // Let's now set up the arguments
        rctx->functionReturnType = get_return_type(gctx);
        rctx->variables["state"] = RuntimeVariableInfo{
                true,
                "state",
                ref
        };
        // This will be treated as a tuple
        auto state_value_reference = (new parser::nodes::ValueReference(Coordinate{0, 0, 0}, "state"))->get();
        parser::NodeList argument_references;
        for (int i = 0; i < arg_types.size(); i++) {
            rctx->variables["_" + std::to_string(i)] = RuntimeVariableInfo{
                    true,
                    "_" + std::to_string(i),
                    arg_types[i]
            };
            argument_references.push_back(
                    (new parser::nodes::ValueReference(Coordinate{0, 0, 0}, "_" + std::to_string(i)))->get());
        }
        auto lctx = gctx->gc.gcnew<LocalContext>(rctx);
        parser::NodePtr full_operation;
        auto first = (new parser::nodes::Subscription(Coordinate{0, 0, 0}, state_value_reference,
                                                      (new parser::nodes::IntegerLiteral({0, 0, 0}, 0))->get()))->get();
        if (is_functional_type(operand_types[0])) {
            first = (new parser::nodes::TupleCall({0, 0, 0}, first, argument_references))->get();
        }
        parser::NodePtr second;
        if (enums::is_binary_op(operation)) {
            second = (new parser::nodes::Subscription(Coordinate{0, 0, 0}, state_value_reference,
                                                      (new parser::nodes::IntegerLiteral({0, 0, 0},
                                                                                         1))->get()))->get();
            if (is_functional_type(operand_types[1])) {
                second = (new parser::nodes::TupleCall({0, 0, 0}, second, argument_references))->get();
            }
        }
        switch (operation) {
            case enums::SimpleOperation::UnaryPlus:
                full_operation = (new parser::nodes::UnaryPlus({0, 0, 0}, first))->get();
                break;
            case enums::SimpleOperation::UnaryMinus:
                full_operation = (new parser::nodes::UnaryMinus({0, 0, 0}, first))->get();
                break;
            case enums::SimpleOperation::Not:
                full_operation = (new parser::nodes::Not({0, 0, 0}, first))->get();
                break;
            case enums::SimpleOperation::Multiplication:
                full_operation = (new parser::nodes::Multiplication({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Division:
                full_operation = (new parser::nodes::Division({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Remainder:
                full_operation = (new parser::nodes::Modulus({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Addition:
                full_operation = (new parser::nodes::Addition({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Subtraction:
                full_operation = (new parser::nodes::Subtraction({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::LeftShift:
                full_operation = (new parser::nodes::LeftShift({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::RightShift:
                full_operation = (new parser::nodes::RightShift({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::LesserThan:
                full_operation = (new parser::nodes::LesserThan({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::GreaterThan:
                full_operation = (new parser::nodes::GreaterThan({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::LesserThanOrEqualTo:
                full_operation = (new parser::nodes::LesserEqual({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::GreaterThanOrEqualTo:
                full_operation = (new parser::nodes::GreaterEqual({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::EqualTo:
                full_operation = (new parser::nodes::EqualTo({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::NotEqualTo:
                full_operation = (new parser::nodes::NotEqualTo({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::And:
                full_operation = (new parser::nodes::And({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Or:
                full_operation = (new parser::nodes::Or({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Xor:
                full_operation = (new parser::nodes::Xor({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Combine:
                full_operation = (new parser::nodes::Combination({0, 0, 0}, first, second))->get();
                break;
            case enums::SimpleOperation::Compose:
                full_operation = (new parser::nodes::TupleCall({0, 0, 0}, first, {second}))->get();
        }
        std::vector<bacteria::FunctionArgument> arguments = {};
        arguments.emplace_back(gctx->global_receiver->get_type(bacteria::BacteriaType::Type::Reference, 0,
                                                               get_cached_type(gctx->global_receiver.get())), "state");
        for (int i = 0; i < arg_types.size(); i++) {
            arguments.emplace_back(arg_types[i]->get_cached_type(gctx->global_receiver.get()), "_" + std::to_string(i));
        }

        auto fn = new bacteria::nodes::Function({0, 0, 0}, cached_function_name, arguments,
                                                get_return_type(gctx)->get_cached_type(
                                                        gctx->global_receiver.get()));
        gctx->global_receiver->receive(fn->get());
        fn->receive((new bacteria::nodes::Return({0, 0, 0}, translate_expression(lctx, full_operation)))->get());
        return cached_function_name;
    }
}