
#ifndef SEPARATE_SEPARATED_PTR_HPP_
#define SEPARATE_SEPARATED_PTR_HPP_

#include <separate/detail/call.hpp>
#include <separate/detail/worker_thread.hpp>
#include <separate/detail/type_traits.hpp>
#include <mutex>
#include <type_traits>
#include <memory>

namespace separated {
    enum policies {
        default_policy  = 0,
        allow_handles   = 1,
    };
}

template<class C, separated::policies Policy = separated::default_policy>
class separated_ptr;
    
template<class C, typename... Args>
separated_ptr<C> make_separated(Args&&...);

template<class C, separated::policies Policy>
class separated_ptr {
public:
    typedef C element_type;
    
    struct data {
        std::mutex mut;
        C obj;
        separate_detail::worker_thread th;

        template<typename... Args>
        explicit data(Args&&... args) : mut(), obj(args...), th() {}
    };
    
    class movable_arg {
    private:
        template<class QualifiedClass, typename FP>
        struct qualified_call {
            typedef separate_detail::call<
                separate_detail::is_command<FP>::value,
                QualifiedClass,
                FP
            > type;
        };
            
    public:
        /* implicit */ movable_arg(separated_ptr p) : data_(p.data_),
                lk_(data_->mut)
        {
            //TODO: th_->sync<int>([] { return 0; });
        }

        //arg& operator=(arg&) = delete;
        movable_arg(movable_arg const& other) = delete;
        movable_arg& operator=(movable_arg const&) = delete;

        // TODO: Consider allowing to use ->* to also access data members
        // (including const support).

        // TODO: Test volatile gives a compiler error (maybe use static_assert for
        // better error). Rationale: std::bind doesn't support volatile anyway...
        // plus volatile and multi-threaded programming don't go along.

        template<typename FP>
        typename std::enable_if<!separate_detail::is_const<FP>::value,
            typename qualified_call<C, FP>::type
        >::type operator->*(FP fp) {
            std::clog << "** operator->* mutable" << std::endl;
            //static_assert(
            //    (Policy & separated::allow_handles) ||
            //            !separate_detail::has_handle<FP>::value,
            //    "separated functions cannot use handles (pointers, references, "
            //    "etc.) as return or parameter types (see `separated::allow_handle` "
            //    "to relax this)"
            //);
            return typename qualified_call<C, FP>::type(
                    data_->th, data_->obj, fp);
        }
        
        template<typename FP>
        typename std::enable_if<separate_detail::is_const<FP>::value,
            typename qualified_call<C const, FP>::type
        >::type operator->*(FP fp) const {
            std::clog << "** operator->* const" << std::endl;
            static_assert(
                (Policy & separated::allow_handles) ||
                        !separate_detail::has_handle<FP>::value,
                "separated functions cannot use handles (pointers, references, "
                "etc.) as return or parameter types (see `separated::allow_handle` "
                "to relax this)"
            );
            return typename qualified_call<C const, FP>::type(
                    data_->th, data_->obj, fp);
        }

    private:
        std::shared_ptr<data> data_;
        std::lock_guard<std::mutex> lk_;
    };

    typedef movable_arg&& arg;

//    template<typename... Args>
//    friend separated_ptr<C> make_separated(Args&&...);

    separated_ptr() : data_() {} // Null ptr.

//TODO: private:
    // Order of following declarations matter for thread safety.

    std::shared_ptr<data> data_;
};

//template<class C>
//class enable_separated_from_this {
//public:
//    separated_ptr<C> separated_from_this() { return p_; }
////TODO: private:
//    separated_ptr<C> p_; // This should be a weak pointer??
//};
//
//template<class C>
//void set_separated_from_this(enable_separated_from_this<C>* base,
//        separated_ptr<C> p) {
//    std::clog << "*** setting enable_separated_from_this" << std::endl;
//    base->p_ = p;
//}
//
//template<class X, class C>
//void set_separated_from_this(X*, separated_ptr<C> p) {}

template<class C>
class enable_separated_from_this {
public:
    separated_ptr<C> separated_from_this() {
        separated_ptr<C> p;
        p.data_ = std::shared_ptr<typename separated_ptr<C>::data>(
                weak_this_data_);
        return p;
    }
    
    separated_ptr<C const> separated_from_this() const {
        separated_ptr<C const> p;
        p.data_ = std::shared_ptr<typename separated_ptr<C const>::data>(
                weak_this_data_);
        return p;
    }

protected:
    enable_separated_from_this() {}
    enable_separated_from_this(enable_separated_from_this const&) {}
    enable_separated_from_this& operator=(enable_separated_from_this const&) {
        return *this;
    }

//TODO:private:
    template<class X, class Y>
    void internal_accept_owner_(
        std::shared_ptr<typename separated_ptr<X>::data> const* ppx,
        typename separated_ptr<Y>::data* py
    ) const {
        if(weak_this_data_.expired()) {
            weak_this_data_ = std::shared_ptr<typename separated_ptr<X>::data>(
                    *ppx, py);
        }
    }

private:
    mutable std::weak_ptr<typename separated_ptr<C>::data> weak_this_data_;
};

template<class X, class Y, class T>
void init_enable_separated_from_this(
    std::shared_ptr<separated_ptr<X>::data> const& ppx,
    Y const& py,
    enable_separated_from_this<T> const* pe
) {
    if(pe != 0) pe->internal_accept_owner(ppx, const_cast<Y*>(py));
}

void init_enable_separated_from_this(...) {}

template<class C, typename... Args>
separated_ptr<C> make_separated(Args&&... args) {
    separated_ptr<C> p;
    p.data_ = std::make_shared<data>(args...);
    init_enable_separated_from_this(p, p.get());
    return p;
}

#endif // #include guard

