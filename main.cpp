#include <iostream>
#include "lexer/lexer.h"
#include "NotImplementedException.h"

int main() {
    auto tokens = cheese::lexer::lex("fn main => u8 entry ==> 1");
    cheese::lexer::output(tokens);
    return 0;
}
