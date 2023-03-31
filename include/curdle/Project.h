//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_PROJECT_H
#define CHEESE_PROJECT_H

#include <filesystem>
#include "parser/parser.h"

namespace cheese::curdle {
    namespace fs = std::filesystem;
    enum class ProjectType {
        Application, // Default, and going to be the first target to test with
        StaticLibrary, // Pain
        DynamicLibrary // Pain
    };
    struct Project {
        fs::path folder;
        fs::path library_folder;
        fs::path standard_library_folder;
        parser::NodePtr root_file; //folder + "/" + main.chs
        ProjectType type;
    };
}

#endif //CHEESE_PROJECT_H
