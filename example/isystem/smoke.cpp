
#include "smoke.hpp" // Warnings enabled for my headers.

#define LCXX_ISYSTEM0 <boost/static_assert.hpp>
#define LCXX_ISYSTEM1 <boost/type_traits.hpp>
#include <lcxx/isystem.hpp> // Include Boost headers above, disabling warnings.

#include <vector> // Warnings already always disable for STL.
#include <memory>

int main() {
    BOOST_STATIC_ASSERT(boost::is_same<a, a>::value);
    return 0;
}

