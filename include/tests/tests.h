//
// Created by Lexi Allen on 9/19/2022.
//

#ifndef CHEESE_TESTS_H
#define CHEESE_TESTS_H
#include <functional>
#include <string>
#include "thirdparty/json.hpp"
namespace cheese::tests {

    typedef std::function<bool()> CheeseTest;
    struct Test;
    extern std::vector<Test*> all_tests;

    struct Test {
        std::string name;
        CheeseTest test;
        bool expects_error = false;
        bool expects_warning = false; //If this is true the previous thing should be true
        Test(std::string name, CheeseTest test, bool expects_error = false, bool expects_warning = false) : name(std::move(name)), test(std::move(test)), expects_error(expects_error), expects_warning(expects_warning) {
            all_tests.push_back(this);
        }
    };
    void run_all_builtin();
    void run_json_tests(std::string filename);
    bool run_single_json_test(nlohmann::json test);
    void gen_pass();
    void gen_fail();
}

#endif //CHEESE_TESTS_H
