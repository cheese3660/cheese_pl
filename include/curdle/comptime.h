//
// Created by Lexi Allen on 3/20/2023.
//


#ifndef CHEESE_COMPTIME_H
#define CHEESE_COMPTIME_H


namespace cheese::curdle {

    struct GlobalContext;
    struct ComptimeValue {

    };

    struct ComptimeContext {
        GlobalContext &ctx;
    };
}
#endif //CHEESE_COMPTIME_H
