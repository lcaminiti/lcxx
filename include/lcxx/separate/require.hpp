
#ifndef LCXX_SEPARATED_REQUIRE_HPP_
#define LCXX_SEPARATED_REQUIRE_HPP_

namespace lcxx { namespace separated {

template<typename Args..., typename F>
void require(separate<Args>::arg... args, F f) {
}

} } // namespace

#endif // #include guard

