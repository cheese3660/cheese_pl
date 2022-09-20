//
// Created by Lexi Allen on 9/19/2022.
//

#ifndef CHEESE_TOOLS_H
#define CHEESE_TOOLS_H
#include <argparse/argparse.hpp>
#include <functional>
#include <vector>
#include <unordered_map>
namespace cheese::tools {
    typedef std::function<int(std::vector<std::string>)> CheeseTool;
    extern std::unordered_map<std::string, CheeseTool> tools;

    argparse::ArgumentParser get_parser(std::string name); //Adds common arguments depending on the tool being run
    void process_common_arguments(argparse::ArgumentParser& parser); //Processes common arguments
}

#endif //CHEESE_TOOLS_H
