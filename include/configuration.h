//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_CONFIGURATION_H
#define CHEESE_CONFIGURATION_H
#include <filesystem>
#include <map>
#include <functional>
namespace cheese::configuration {
    enum class ReleaseMode {
        Debug,
        Release,
    };
    void default_error_output_handler(std::string);


    extern std::function<void(std::string)> error_output_handler;


    extern bool use_escape_sequences;
    extern ReleaseMode release_mode;
    extern std::filesystem::path project_directory;
    extern std::filesystem::path std_directory; //Where the standard library is located
    extern std::map<std::string, std::filesystem::path> lib_directories; //A mapping of library names to their directories
                                                                         //Usually installed locally
                                                                         //Dependency resolution is going to be a bitch ... NPM anyone?
    extern bool log_errors; //Whether errors should be logged, this is set to false when running the compiler tests for things that don't expect errors for example
    extern bool warnings_are_errors; //Whether warnings should be treated as errors
    extern bool die_on_first_error; //Whether the compiler should die on the first error, used by the testing system to test for errors
    //Add the target information into here once it's feasible to do so

    void setup_escape_sequences();
} // cheese::configuration

#endif //CHEESE_CONFIGURATION_H
