//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_NOTIMPLEMENTEDEXCEPTION_H
#define CHEESE_NOTIMPLEMENTEDEXCEPTION_H
#include <stdexcept>
#include <string>

namespace cheese {

    class NotImplementedException : public std::logic_error {

    public:

        // Construct with given error message:
        NotImplementedException(const char *message)
                : logic_error(message) {
        }
    };

} // cheese

#define NOT_IMPL do { \
    std::string file_name = __FILE__; \
    std::string func_name = __func__; \
    std::string line = std::to_string(__LINE__);     \
    std::string combined = file_name+":"+line+": "+func_name+" is not implemented"; \
    throw NotImplementedException(combined.c_str());\
} while (0);
#define NOT_IMPL_FOR(X) do { \
    std::string file_name = __FILE__; \
    std::string func_name = __func__; \
    std::string line = std::to_string(__LINE__);     \
    std::string combined = file_name+":"+line+": "+func_name+" is not implemented for "+X; \
    throw NotImplementedException(combined.c_str());\
} while (0);
#endif //CHEESE_NOTIMPLEMENTEDEXCEPTION_H
