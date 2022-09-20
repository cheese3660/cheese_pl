//
// Created by Lexi Allen on 9/19/2022.
//
#include "lexer/lexer.h"
#include "vterm.h"
#include "error.h"
#include "tests/tests.h"
#include <iostream>
namespace cheese::tests::lexer {
    bool all_tokens_have_repr_test() {
        std::uint32_t tBegin = 0;
        auto tEnd = static_cast<std::uint32_t>(cheese::lexer::TokenType::EoF);
        bool passed = true;
        for (std::uint32_t i = tBegin; i <= tEnd; i++) {
            auto t = static_cast<cheese::lexer::TokenType>(i);
            std::cout << "\ttesting token " << i << " ... ";
            std::string name;
            try {
                name = cheese::lexer::name_of(t);
                gen_pass();
                std::cout << " -  " << name << '\n';
            } catch (std::exception& e)
            {
                gen_fail();
                passed = false;
            }
        }
        return passed;
    }

    [[maybe_unused]] Test all_tokens_have_repr(
        "all tokens should have a string representation",
        all_tokens_have_repr_test
    ); //Tests if all tokens have a string representation

    //Do all the error testing first
    bool comment_todo_warning_test() {
        bool passed = true;
        std::string warning_comment_single_line = "//TODO ...";
        std::string warning_comment_multiline = "/*implemented\nTODO ...*/";
        std::cout << "\ttesting single line comment...";
        try {
            cheese::lexer::lex(warning_comment_single_line);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::TodoFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            std::cout << '\t' << e.what() << '\n';
        }
        std::cout << "\ttesting multi line comment...";
        try {
            cheese::lexer::lex(warning_comment_multiline);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::TodoFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            passed = false;
            std::cout << '\t' << e.what() << '\n';
        }

        return passed;
    }

    [[maybe_unused]] Test comment_todo_warning(
        "comments with TODO in them should give a warning",
        comment_todo_warning_test,
        true,
        true
    );

    bool comment_hack_warning_test() {
        bool passed = true;
        std::string warning_comment_single_line = "//HACK ...";
        std::string warning_comment_multiline = "/*implemented\nHACK ...*/";
        std::cout << "\ttesting single line comment...";
        try {
            cheese::lexer::lex(warning_comment_single_line);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::HackFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            std::cout << '\t' << e.what() << '\n';
        }
        std::cout << "\ttesting multi line comment...";
        try {
            cheese::lexer::lex(warning_comment_multiline);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::HackFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                passed = false;
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            passed = false;
            std::cout << '\t' << e.what() << '\n';
        }

        return passed;
    }

    [[maybe_unused]] Test comment_hack_warning(
            "comments with HACK in them should give a warning",
            comment_hack_warning_test,
            true,
            true
            );
    bool comment_fixme_warning_test() {
        bool passed = true;
        std::string warning_comment_single_line = "//FIXME ...";
        std::string warning_comment_multiline = "/*implemented\nFIXME ...*/";
        std::cout << "\ttesting single line comment...";
        try {
            cheese::lexer::lex(warning_comment_single_line);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::FixmeFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            passed = false;
            std::cout << '\t' << e.what() << '\n';
        }
        std::cout << "\ttesting multi line comment...";
        try {
            cheese::lexer::lex(warning_comment_multiline);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::FixmeFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                passed = false;
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            passed = false;
            std::cout << '\t' << e.what() << '\n';
        }
        return passed;
    }

    [[maybe_unused]] Test comment_fixme_warning(
            "comments with FIXME in them should give a warning",
            comment_fixme_warning_test,
            true,
            true
    );
    bool comment_bug_warning_test() {
        bool passed = true;
        std::string warning_comment_single_line = "//BUG ...";
        std::string warning_comment_multiline = "/*implemented\bBUG ...*/";
        std::cout << "\ttesting single line comment...";
        try {
            cheese::lexer::lex(warning_comment_single_line);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::BugFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                passed = false;
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            passed = false;
            std::cout << '\t' << e.what() << '\n';
        }
        std::cout << "\ttesting multi line comment...";
        try {
            cheese::lexer::lex(warning_comment_multiline);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::BugFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                passed = false;
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            std::cout << '\t' << e.what() << '\n';
        }
        return passed;
    }

    [[maybe_unused]] Test comment_bug_warning(
            "comments with BUG in them should give a warning",
            comment_bug_warning_test,
            true,
            true
    );

    [[maybe_unused]] bool comment_xxx_warning_test() {
        bool passed = true;
        std::string warning_comment_single_line = "//XXX ...";
        std::string warning_comment_multiline = "/*implemented\bXXX ...*/";
        std::cout << "\ttesting single line comment...";
        try {
            cheese::lexer::lex(warning_comment_single_line);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::XXXFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                passed = false;
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            std::cout << '\t' << e.what() << '\n';
        }
        std::cout << "\ttesting multi line comment...";
        try {
            cheese::lexer::lex(warning_comment_multiline);
            gen_fail();
            passed = false;
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::XXXFoundInComment) {
                gen_pass();
                std::cout << '\n';
            } else {
                gen_fail();
                passed = false;
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            gen_fail();
            passed = false;
            std::cout << '\t' << e.what() << '\n';
        }
        return passed;
    }

    [[maybe_unused]] Test comment_xxx_warning(
            "comments with BUG in them should give a warning",
            comment_bug_warning_test,
            true,
            true
    );
    bool unterminated_string_test() {
        std::string str = "\"This is unterminated";
        try {
            cheese::lexer::lex(str);
            std::cout << '\t';
            gen_fail();
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::UnterminatedStringLiteral) {
                std::cout << '\t';
                gen_pass();
                std::cout << '\n';
                return true;
            } else {
                std::cout << '\t';
                gen_fail();
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            std::cout << '\t';
            gen_fail();
            std::cout << '\t' << e.what() << '\n';
        }
        return false;
    }

    [[maybe_unused]] Test unterminated_string(
            "unterminated strings should cause an error",
            unterminated_string_test,
            true
            );

    bool unterminated_character_test() {
        std::string str = "'a";
        try {
            cheese::lexer::lex(str);
            std::cout << '\t';
            gen_fail();
        } catch (error::CompilerError& compilerError) {
            if (compilerError.code == error::ErrorCode::UnterminatedCharacterLiteral) {
                std::cout << '\t';
                gen_pass();
                std::cout << '\n';
                return true;
            } else {
                std::cout << '\t';
                gen_fail();
                std::cout << '\t' << compilerError.what() << '\n';
            }
        } catch (std::exception& e) {
            std::cout << '\t';
            gen_fail();
            std::cout << '\t' << e.what() << '\n';
        }
        return false;
    }

    [[maybe_unused]] Test unterminated_character(
            "unterminated character literals should cause an error",
            unterminated_character_test,
            true
            );
    // Then start doing validations of tests
    //Test locations;

}