
#ifndef LCXX_DEBUG_HPP_
#define LCXX_DEBUG_HPP_

#ifdef LCXX_DEBUG
    #include <lcxx/thread/threadsafe_out.hpp>
    #include <iostream>

    #define LCXX_DLOG(outputs) \
        LCXX_THREADSAFE_OUT(std::clog, outputs)
#else
    #define LCXX_DLOG(outputs) /* nothing */
#endif

#endif // #include guard

