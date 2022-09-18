//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_COORDINATE_H
#define CHEESE_COORDINATE_H
#include <cstdint>
#include <string>
#include <vector>
namespace cheese {
    extern std::vector<std::string> filenames;

    std::size_t getFileIndex(std::string filename);

    struct Coordinate {
        std::uint32_t line_number;
        std::uint32_t column_number;
        std::size_t file_index;
        std::string toString();
    };
}


#endif //CHEESE_COORDINATE_H
