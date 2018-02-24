
#ifndef LCXX_LOCALVAR_HPP_
#define LCXX_LOCALVAR_HPP_

#include <type_traits>

namespace lcxx {

struct undeclare_localvar {};

template<typename X, typename G = undeclare_localvar>
struct is_localvar :
    std::integral_constant<bool, !std::is_same<X, undeclare_localvar>::value>
{
    static_assert(std::is_same<G, undeclare_localvar>::value,
            "global variable is not declared of type `undeclare_localvar`");
};

template<typename X, typename G = undeclare_localvar>
inline constexpr bool is_localvar_v = is_localvar<X, G>::value;

} // namespace

#define LCXX_IS_LOCALVAR(name) \
    (lcxx::is_localvar<decltype(name), decltype(::name)>::value)

#endif // #include guard

