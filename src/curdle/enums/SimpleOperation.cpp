//
// Created by Lexi Allen on 9/30/2023.
//
#include "curdle/enums/SimpleOperation.h"

std::string cheese::curdle::enums::get_op_string(SimpleOperation op) {
    switch (op) {
        case SimpleOperation::Addition:
        case SimpleOperation::UnaryPlus:
            return "+";
        case SimpleOperation::Subtraction:
        case SimpleOperation::UnaryMinus:
            return "-";
        case SimpleOperation::Not:
            return "not ";
        case SimpleOperation::Multiplication:
            return "*";
        case SimpleOperation::Division:
            return "/";
        case SimpleOperation::Remainder:
            return "%";
        case SimpleOperation::LeftShift:
            return "<<";
        case SimpleOperation::RightShift:
            return ">>";
        case SimpleOperation::LesserThan:
            return "<";
        case SimpleOperation::GreaterThan:
            return ">";
        case SimpleOperation::LesserThanOrEqualTo:
            return "<=";
        case SimpleOperation::GreaterThanOrEqualTo:
            return ">=";
        case SimpleOperation::EqualTo:
            return "==";
        case SimpleOperation::NotEqualTo:
            return "!=";
        case SimpleOperation::And:
            return " and ";
        case SimpleOperation::Or:
            return " or ";
        case SimpleOperation::Xor:
            return " xor ";
        case SimpleOperation::Combine:
            return "&";
        case SimpleOperation::Compose:
            return "|";
    }
}

bool cheese::curdle::enums::is_binary_op(cheese::curdle::enums::SimpleOperation op) {
    switch (op) {
        case enums::SimpleOperation::UnaryPlus:
        case enums::SimpleOperation::UnaryMinus:
        case enums::SimpleOperation::Not:
            return false;
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
            return true;
    }
}

