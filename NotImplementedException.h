//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_NOTIMPLEMENTEDEXCEPTION_H
#define CHEESE_NOTIMPLEMENTEDEXCEPTION_H
#include <stdexcept>

namespace cheese {

    class NotImplementedException : public std::logic_error {

    public:

        // Construct with given error message:
        NotImplementedException(const char *message, const char *error = "Functionality not yet implemented!")
                : logic_error(message) {
        }
    };

} // cheese

#endif //CHEESE_NOTIMPLEMENTEDEXCEPTION_H
