
#ifndef LCXX_ATOMIC_OUT_HPP_
#define LCXX_ATOMIC_OUT_HPP_

#include <boost/preprocessor/cat.hpp>
#include <sstream>

// Print on single unbroken line even from multiple threads for std::cout, etc.
#define LCXX_ATOMIC_OUT(ostream, outputs) \
    { \
        std::ostringstream BOOST_PP_CAT(_lcxx_log_, __LINE__); \
        BOOST_PP_CAT(_lcxx_log_, __LINE__) << outputs; \
        ostream << BOOST_PP_CAT(_lcxx_log_, __LINE__).str(); \
    }

#endif // #include guard

