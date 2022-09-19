//
// Created by Lexi Allen on 9/18/2022.
//

#include "configuration.h"

namespace cheese::configuration {
    bool use_escape_sequences = true;
    ReleaseMode release_mode = ReleaseMode::Debug;
    std::filesystem::path project_directory;
    std::filesystem::path std_directory;
    std::map<std::string, std::filesystem::path> lib_directories;


} // cheese::configuration