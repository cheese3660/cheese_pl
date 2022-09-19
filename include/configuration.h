//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_CONFIGURATION_H
#define CHEESE_CONFIGURATION_H
#include <filesystem>
#include <map>

namespace cheese::configuration {
    enum class ReleaseMode {
        Debug,
        Release,
    };
    extern bool use_escape_sequences;
    extern ReleaseMode release_mode;
    extern std::filesystem::path project_directory;
    extern std::filesystem::path std_directory; //Where the standard library is located
    extern std::map<std::string, std::filesystem::path> lib_directories; //A mapping of library names to their directories
                                                                         //Usually installed locally
                                                                         //Dependency resolution is going to be a bitch ... NPM anyone?
    //Add the target information into here once it's feasible to do so


} // cheese::configuration

#endif //CHEESE_CONFIGURATION_H
