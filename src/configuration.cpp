//
// Created by Lexi Allen on 9/18/2022.
//

#include "configuration.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
namespace cheese::configuration {
    bool use_escape_sequences = true;
    ReleaseMode release_mode = ReleaseMode::Debug;
    std::filesystem::path project_directory;
    std::filesystem::path std_directory;
    std::map<std::string, std::filesystem::path> lib_directories;
    bool log_errors = true;
    bool warnings_are_errors = false;
    bool die_on_first_error = false;

    void setup_escape_sequences() {
#ifdef WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) {
            use_escape_sequences = false;
            return;
        }
        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) {
            use_escape_sequences = false;
            return;
        }
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode)) {
            use_escape_sequences = false;
            return;
        }
        system(("chcp " + std::to_string(CP_UTF8)).c_str());
#else
        if (!isatty(STDOUT_FILENO)) {
            use_escape_sequences = false;
        }
#endif
    }
} // cheese::configuration