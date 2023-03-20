//
// Created by Lexi Allen on 9/18/2022.
//

#include "Coordinate.h"
#include <ranges>
#include <algorithm>

namespace cheese {
    std::vector<std::string> filenames{};

    std::size_t getFileIndex(const std::string &filename) {
        for (std::size_t i = 0; i < filenames.size(); i++) {
            if (filename == filenames[i]) {
                return i;
            }
        }
        filenames.push_back(filename);
        return filenames.size() - 1;
    }

    std::string Coordinate::toString() const {
        return filenames[file_index] + ":" + std::to_string(line_number) + ":" + std::to_string(column_number);
    }

    bool Coordinate::operator!=(const Coordinate &other) const {
        return file_index != other.file_index && line_number != other.line_number &&
               column_number != other.column_number;
    }
}