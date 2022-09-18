//
// Created by Lexi Allen on 9/18/2022.
//

#include "Coordinate.h"
namespace cheese {
    std::vector<std::string> filenames{};

    std::string Coordinate::toString() {
        return filenames[file_index] + ":" + std::to_string(line_number) + ":" + std::to_string(column_number);
    }
}