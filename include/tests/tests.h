//
// Created by Lexi Allen on 9/19/2022.
//
#ifndef CHEESE_NO_SELF_TESTS
#ifndef CHEESE_TESTS_H
#define CHEESE_TESTS_H

#include <functional>
#include <string>
#include <optional>
#include "thirdparty/json.hpp"

namespace cheese::tests {

    enum class SingleResult {
        Pass,
        Fail,
        Skip, //Skip is used when a prior test failed that leaves this test in an invalid state, or the feature isn't implementable yet
    };

    struct TestResults {
        std::uint32_t pass = 0;
        std::uint32_t fail = 0;
        std::uint32_t skip = 0;

        TestResults operator+(TestResults other) const {
            return TestResults{pass + other.pass, fail + other.fail, skip + other.skip};
        }

        void display_results(std::uint32_t nesting) const; //Displays #tests ran, #passed, #failed, and the percentage
    };

    //Do a better testing system, that allows for sections and subsections
    struct TestSection;
    struct TestCase;

    struct SectionMember {
        bool is_subsection;
        union {
            TestSection *subsection;
            TestCase *test_case;
        };

        TestResults run(int nesting) const;
    };

    typedef std::function<void()> TestSetupDestroy;

    struct TestSection {
        std::vector<SectionMember> members;
        std::optional<TestSetupDestroy> setup;
        std::optional<TestSetupDestroy> destroy;
        std::string section_name;

        TestSection(std::string name,
                    std::uint32_t priority); //priority is used for determining the global order of tests to be run
        TestSection(std::string name, TestSection *parent);


        void add(TestSection *subsection);

        void add(TestCase *subCase);

        TestResults run(int nesting); //It will then display the results at the bottom

    };

    typedef std::function<SingleResult(std::uint32_t)> TestCaseWithNesting;


    struct TestCase {
        TestCaseWithNesting test_case;
        std::string description;
        bool expects_error = false;
        bool expects_warning = false; //If this is true the previous thing should be true
        TestCase(TestSection *parent, std::string description, TestCaseWithNesting test_case, bool expects_error,
                 bool expects_warning);

        TestResults run(int nesting); //Will do the entire display the test name, and then the results
    };

    void run_all_builtin();

    void run_json_tests(const std::string &filename);

    bool run_single_json_test(nlohmann::json test);

    void gen_pass(std::uint32_t nesting = 0);

    void gen_fail(std::uint32_t nesting = 0);

    void gen_skip(std::uint32_t nesting = 0);

    void test_output_message(std::uint32_t nesting,
                             std::string message); //Will properly indent and everything depending on the level of nesting
}

using namespace cheese::tests; //Because all the testing macros require this

#define TEST_NAME_MERGE(a, b)  a##b
#define SECTION_NAMESPACE(a) TEST_NAME_MERGE(section_,a)
#define SECTION_STRUCTURE(a) TEST_NAME_MERGE(__section_,a)
#define SUBSECTION_NAMESPACE(a) TEST_NAME_MERGE(subsection_,a)
#define SUBSECTION_STRUCTURE(a) TEST_NAME_MERGE(__subsection_,a)
#define TEST_NAME(a) TEST_NAME_MERGE(test_,a)
#define TEST_OBJECT(a) TEST_NAME_MERGE(__test_,a)
#define SETUP_NAME(a) TEST_NAME_MERGE(setup_,a)
#define SETUP_OBJECT(a) TEST_NAME_MERGE(__setup_,a)
#define SETUP_STRUCTURE(a) TEST_NAME_MERGE(_setup_,a)
#define DESTROY_NAME(a) TEST_NAME_MERGE(destroy_,a)
#define DESTROY_OBJECT(a) TEST_NAME_MERGE(__destroy_,a)
#define DESTROY_STRUCTURE(a) TEST_NAME_MERGE(_destroy_,a)
#define OFFSET_NAME_1(a) TEST_NAME_MERGE(_1_, a)
#define OFFSET_NAME_2(a) TEST_NAME_MERGE(_2_, b)
#define OFFSET_NAME_3(a) TEST_NAME_MERGE(_3_, c)
#define GENERATED(a) TEST_NAME_MERGE(generated_,a)


#define __TEST_SECTION(line, section_description, section_ordering)                                                          \
namespace SECTION_NAMESPACE(line){                                                               \
struct SECTION_STRUCTURE(line) {                                                       \
    static TestSection Section;             \
};                                                                                                                           \
using __next_section = SECTION_STRUCTURE(line);\
TestSection __next_section::Section = TestSection{section_description, static_cast<std::uint32_t>(section_ordering)};                                                                                                                             \
using __current_section = __next_section;

#define __TEST_SUBSECTION(line, section_description) namespace SUBSECTION_NAMESPACE(line) {\
    struct SUBSECTION_STRUCTURE(line) {                                                                    \
        static TestSection Section;                                                                                                  \
    };                                                                                     \
    using __next_section = SUBSECTION_STRUCTURE(line);                                      \
    TestSection __next_section::Section = TestSection{section_description, &__current_section::Section};\
    using __current_section = __next_section;
#define __TEST_CASE(line, description, err, warn) SingleResult TEST_NAME(line)(std::uint32_t); TestCase TEST_OBJECT(line)(&__current_section::Section,description,TEST_NAME(line),err,warn); SingleResult TEST_NAME(line)(std::uint32_t __nesting)
#define TEST_CASE(description) __TEST_CASE(__LINE__,description,false,false)
#define TEST_ERR(description) __TEST_CASE(__LINE__,description,true,false)
#define TEST_WARN(description) __TEST_CASE(__LINE__,description,true,true)

#define __TEST_SETUP(line) void SETUP_NAME(line)(); struct SETUP_STRUCTURE(line){ SETUP_STRUCTURE(line)() {__current_section::Section.setup = SETUP_NAME(line);}} SETUP_OBJECT(line); void SETUP_NAME(line)()
#define TEST_SETUP __TEST_SETUP(__LINE__)
#define __TEST_DESTROY(line) void DESTROY_NAME(line)(); struct DESTROY_STRUCTURE(line){ DESTROY_STRUCTURE(line)() {__current_section::Section.destroy = DESTROY_NAME(line);}} DESTROY_OBJECT(line); void DESTROY_NAME(line)()
#define TEST_DESTROY __TEST_DESTROY(__LINE__)


//implicitly defines a destructor for the section that will destroy all generated tests
//And then defines a setup that will define all tests when ran
#define __TEST_GENERATOR(line) std::vector<TestCase*> __generated_tests; TEST_DESTROY {for (auto t : __generated_tests) {delete t;} __generated_tests.clear();}; TEST_SETUP
#define TEST_GENERATOR __TEST_GENERATOR(__LINE)

//Generators always capture by copy
#define TEST_GEN_BEGIN(description) __generated_tests.push_back(new TestCase(&__current_section::Section,description, [=](std::uint32_t __nesting) -> SingleResult {
#define TEST_GEN_END }, false,false));
#define TEST_GEN_END_ERR }, true, false));
#define TEST_GEN_END_WARN }, true, false));

#define MESSAGE(X) test_output_message(__nesting, (X))
#define NEWLINE(X) (std::string(X) + '\n')

#define TEST_SECTION(section_description, section_ordering) __TEST_SECTION(__LINE__, section_description, section_ordering)
#define TEST_SUBSECTION(section_description) __TEST_SUBSECTION(__LINE__, section_description)
//The current test failed with a message
//This will print the message on the lines afterwards properly indented
#define TEST_FAIL_MESSAGE(message) do {gen_fail(__nesting); test_output_message(__nesting, (message)); return SingleResult::Fail;} while(0)
#define TEST_FAIL do {gen_fail(__nesting); return SingleResult::Fail;} while(0)

#define TEST_SKIP_MESSAGE(message) do {gen_skip(__nesting); test_output_message(__nesting, (message) return SingleResult::Skip;)} while (0)
#define TEST_SKIP do {gen_skip(__nesting); return SingleResult::Skip;} while(0)

#define TEST_PASS do {gen_pass(__nesting); return SingleResult::Pass;} while(0)

#define TEST_ASSERT(condition) do {if ((condition)) TEST_PASS; else TEST_FAIL;} while(0)
#define TEST_ASSERT_MESSAGE(condition, message) do {if ((condition)) TEST_PASS; else TEST_FAIL_MESSAGE(message);} while(0)
#define TEST_ASSERT_CONTINUE(condition) do {if (!(condition)) TEST_FAIL;} while(0)
#define TEST_ASSERT_CONTINUE_MESSAGE(condition, message) do {if (!(condition)) TEST_FAIL_MESSAGE(message);} while(0)


#define TEST_ASSERT_EQ(X, Y) TEST_ASSERT((X) == (Y))
#define TEST_ASSERT_NEQ(X, Y) TEST_ASSERT((X) != (Y))
#define TEST_ASSERT_EQ_MESSAGE(X, Y, message) TEST_ASSERT_MESSAGE((X) == (Y),message)
#define TEST_ASSERT_NEQ_MESSAGE(X, Y, message) TEST_ASSERT_MESSAGE((X) != (Y),message)
#define TEST_ASSERT_EQ_CONTINUE(X, Y) TEST_ASSERT_CONTINUE((X) == (Y))
#define TEST_ASSERT_NEQ_CONTINUE(X, Y) TEST_ASSERT_CONTINUE((X) != (Y))
#define TEST_ASSERT_EQ_CONTINUE_MESSAGE(X, Y, message) TEST_ASSERT_CONTINUE_MESSAGE((X) == (Y),message)
#define TEST_ASSERT_NEQ_CONTINUE_MESSAGE(X, Y, message) TEST_ASSERT_CONTINUE_MESSAGE((X) != (Y),message)
#define TEST_TRY(expr) do {try { expr; } catch (cheese::error::CompilerError& c) {TEST_FAIL_MESSAGE(NEWLINE(c.what()));} catch (std::exception& e) {TEST_FAIL_MESSAGE(NEWLINE(e.what()));}} while(0)
#define TEST_TRY_MESSAGE(expr, message) do {try { expr; } catch (cheese::error::CompilerError& c) {TEST_FAIL_MESSAGE(std::string(message) + c.what() + "\n");} catch (std::exception& e) {TEST_FAIL_MESSAGE(std::string(message) + NEWLINE(e.what()));}} while(0)
#define TEST_EXPECT(expr, expected_error) do { try {expr; TEST_FAIL; } catch (cheese::error::CompilerError& c) {if (c.code == (expected_error)) {TEST_PASS;} else {TEST_FAIL_MESSAGE(NEWLINE(c.what()));}} catch (std::exception& e) {TEST_FAIL_MESSAGE(NEWLINE(e.what()));}} while(0)
#define TEST_EXPECT_MESSAGE(expr, expected_error, message)  do { try {expr; TEST_FAIL_MESSAGE(message); } catch (cheese::error::CompilerError& c) {if (c.code == (expected_error)) {TEST_PASS;} else {TEST_FAIL_MESSAGE(std::string(message) + NEWLINE(c.what()));}} catch (std::exception& e) {TEST_FAIL_MESSAGE(std::string(message) + NEWLINE(e.what()));}} while(0)

#define TEST_SKIP_IF(X) do {if (X) TEST_SKIP;} while (0)
#define TEST_SKIP_IF_MESSAGE(X, message) do {if (X) TEST_SKIP_MESSAGE(message);} while 0


#define TEST_SETUP_ERROR_OUTPUT configuration::error_output_handler = [__nesting](const std::string& msg) { test_output_message(__nesting,msg);}


#define TEST_END }


#endif //CHEESE_TESTS_H
#endif //CHEESE_NO_SELF_TESTS