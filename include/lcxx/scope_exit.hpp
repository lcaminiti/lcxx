
#ifndef LCXX_SCOPE_EXIT_HPP_
#define LCXX_SCOPE_EXIT_HPP_

#include <functional>

namespace lcxx {

class scope_exit {
public:
    template<typename F>
    explicit scope_exit(F f) : f_(f) {}
    
    ~scope_exit() noexcept(false) { f_(); }

private:
    std::function<void ()> f_;
};

} // namespace

#endif // #include guard

