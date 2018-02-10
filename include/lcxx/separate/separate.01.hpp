
#ifndef SEPARATE_SEPARATE_HPP_
#define SEPARATE_SEPARATE_HPP_

#include <separate/detail/call.hpp>
#include <separate/detail/worker_thread.hpp>
#include <separate/detail/type_traits.hpp>
#include <type_traits>

namespace separated {
    enum policies {
        default_policy  = 0,
        allow_handles   = 1,
    };
}

template<class Class, separated::policies Policy = separated::default_policy>
class separate {
private:
    template<class QualifiedClass, typename FuncPtr>
    struct qualified_call {
        typedef separate_detail::call<
            separate_detail::is_command<FuncPtr>::value,
            QualifiedClass,
            FuncPtr
        > type;
    };
        
public:
    typedef Class class_type;
    
    template<typename... Args>
    explicit separate(Args&&... args) : obj_(args...) {
        // TODO: static_assert that Args have no handles.
    }

    // TODO: Consider allowing to use ->* to also access data members
    // (including const support).

    // TODO: Test volatile gives a compiler error (maybe use static_assert for
    // better error). Rationale: std::bind doesn't support volatile anyway...
    // plus volatile and multi-threaded programming don't go along.

    template<typename FuncPtr>
    typename std::enable_if<!separate_detail::is_const<FuncPtr>::value,
        typename qualified_call<Class, FuncPtr>::type
    >::type operator->*(FuncPtr fp) {
        std::cout << "** separate->* mutable" << std::endl;
        static_assert(
            (Policy & separated::allow_handles) ||
                    !separate_detail::has_handle<FuncPtr>::value,
            "separated functions cannot use handles (pointers, references, "
            "etc.) as return or parameter types (see `separated::allow_handle` "
            "to relax this)"
        );
        return typename qualified_call<Class, FuncPtr>::type(
                thread_, obj_, fp);
    }
    
    template<typename FuncPtr>
    typename std::enable_if<separate_detail::is_const<FuncPtr>::value,
        typename qualified_call<Class const, FuncPtr>::type
    >::type operator->*(FuncPtr fp) const {
        std::cout << "** separate->* const" << std::endl;
        static_assert(
            (Policy & separated::allow_handles) ||
                    !separate_detail::has_handle<FuncPtr>::value,
            "separated functions cannot use handles (pointers, references, "
            "etc.) as return or parameter types (see `separated::allow_handle` "
            "to relax this)"
        );
        return typename qualified_call<Class const, FuncPtr>::type(
                thread_, obj_, fp);
    }

private:
    // Order of following declarations matter for thread safety.
    Class obj_;
    mutable separate_detail::worker_thread thread_;
};

#define SEPARATED(s, f) (s->*&decltype(s)::class_type::f)

#endif // #include guard

