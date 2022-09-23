//
// Created by Lexi Allen on 9/21/2022.
//

#ifndef CHEESE_PARSER_H
#define CHEESE_PARSER_H
#include <memory>
#include "node.h"
#include "../lexer/lexer.h"
namespace cheese::parser {
   NodePtr parse(std::vector<lexer::Token>& tokens);
}

//essentially (X is T NAME) from C#
#define IS(X,T,NAME) (auto NAME = std::dynamic_pointer_cast<T>(X); NAME)

#endif //CHEESE_PARSER_H
