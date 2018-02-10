
#ifndef LXX_SCOPE_EXIT_HPP_
#define LXX_SCOPE_EXIT_HPP_

#include <functional>

namespace lxx {

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

