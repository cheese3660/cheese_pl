//
// Created by Lexi Allen on 5/28/2023.
//
#include "util/json_template.h"
#include "fstream"
#include "sstream"
#include <regex>
#include <unordered_map>
#include <unordered_set>

namespace cheese::util {


    void replace_in_place(std::string &subject, const std::string &search,
                          const std::string &replace) {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    nlohmann::json get_template(const nlohmann::json &template_object, const std::filesystem::path &template_path) {
        std::stringstream ss{};
        std::ifstream templ{template_path / (template_object[":template"].get<std::string>() + ".json")};
        ss << templ.rdbuf();
        auto string = ss.str();
        for (auto &[key, value]: template_object.items()) {
            auto v = value;
            templatify(v, template_path);
            auto dump = v.dump();
            replace_in_place(string, "\"@" + key + "\"@", dump);
            if (value.is_string()) {
                replace_in_place(string, "%" + key + "%", v.get<std::string>());
            }
        }
        return nlohmann::json::parse(string);
    }

    [[noreturn]] size_t templatify_base(nlohmann::json &json, const std::filesystem::path &template_path) {
        if (json.is_object()) {
            if (json.contains(":template")) {
                json = get_template(json, template_path);
                return 1;
            } else if (json.contains(":match")) {
                auto match = json[":match"];
                if (json.contains(match)) {
                    json = json[match];
                } else if (json.contains(":default")) {
                    json = json[":default"];
                } else {
                    json = ":remove";
                }
                return 1;
            } else {
                size_t sum = 0;
                std::unordered_set<std::string> to_remove;
                std::unordered_map<std::string, nlohmann::json> to_insert;
                for (auto &[key, value]: json.items()) {
                    sum += templatify_base(value, template_path);
                    if (value.is_object()) {
                        if (value.contains(":insert")) {
                            auto ins = value[":insert"];
                            for (auto &[key2, value2]: ins.items()) {
                                to_insert[key2] = value2;
                            }
                            sum += 1;
                        }
                    } else if (value == ":remove") {
                        to_remove.insert(key);
                        sum += 1;
                    }
                }
                for (auto &name: to_remove) {
                    json.erase(name);
                }
                for (auto &[key, value]: to_insert) {
                    json[key] = value;
                }
                return sum;
            }
        } else if (json.is_array()) {
            size_t sum = 0;
            ptrdiff_t i = 0;
            std::vector<std::pair<ptrdiff_t, nlohmann::json>> insertions;
            for (auto &v: json) {
                sum += templatify_base(v, template_path);
                if (v.is_object()) {
                    if (v.contains(":insert")) {
                        insertions.emplace_back(i, v[":insert"]);
                        sum += 1;
                    }
                } else if (v == ":remove") {
                    insertions.emplace_back(i, nlohmann::json::array());
                    sum += 1;
                }
                i += 1;
            }
            for (ptrdiff_t j = insertions.size() - 1; j >= 0; j--) {
                auto insertion = insertions[j];
                json.erase(insertion.first);
                auto where = insertion.first;
                for (ptrdiff_t k = insertion.second.size(); k >= 0; k--) {
                    if (where == json.size()) {
                        json.push_back(insertion.second[i]);
                    } else {
                        json.insert(json.begin() + where, insertion.second[i]);
                    }
                }
            }
            return sum;
        } else if (json.is_string()) {
            auto as_string = json.get<std::string>();
            if (as_string.starts_with(":template!")) {
                auto t = as_string.substr(9);
                std::stringstream ss{};
                std::ifstream templ{template_path / (t + ".json")};
                ss << templ.rdbuf();
                auto string = ss.str();
                json = nlohmann::json::parse(string);
                return 1;
            }
        }
        return 0;
    }

    void templatify(nlohmann::json &json, const std::filesystem::path &template_path) {
        try {
            size_t count;
            do {
                count = templatify_base(json, template_path);
            } while (count > 0);
        } catch (const std::exception &e) {
            throw;
        }
    }
}