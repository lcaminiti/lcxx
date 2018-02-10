
#ifndef SEPARATE_SEPARATED_PTR_HPP_
#define SEPARATE_SEPARATED_PTR_HPP_

#include <separate/detail/call.hpp>
#include <separate/detail/worker_thread.hpp>
#include <separate/detail/type_traits.hpp>
#include <separate/detail/debug.hpp>
#include <mutex>
#include <type_traits>
#include <memory>
#include <typeinfo>
#include <cassert>

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
    
    struct data : std::enable_shared_from_this<data> {
        mutable std::mutex mut;
        C obj;
        mutable separate_detail::worker_thread th;

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
            SEPARATE_DETAIL_DLOG("operator->* mutable");
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
            SEPARATE_DETAIL_DLOG("operator->* const");
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

    separated_ptr() : data_() {} // Null ptr.
    std::shared_ptr<data> data_;
};

template<class C>
class enable_separated_from_this {
public:
    separated_ptr<C> separated_from_this() {
        separated_ptr<C> p;
        p.data_ = std::shared_ptr<typename separated_ptr<C>::data>(
                weak_this_data_);
        assert(p.data_);
        return p;
    }
    
    //separated_ptr<C const> separated_from_this() const {
    //    separated_ptr<C const> p;
    //    assert(this_);
    //    p.data_ = this_->data_->shared_from_this();
    //    return p;
    //}

    template<class X, class Y>
    void accept_owner_(std::shared_ptr<typename separated_ptr<X>::data> const*
            ppx, Y* py) {
        if(weak_this_data_.expired()) {
            weak_this_data_ = std::shared_ptr<typename separated_ptr<C>::data>(
                    *ppx, py);
        }
    }

private:
    mutable std::weak_ptr<typename separated_ptr<C>::data> weak_this_data_;
};

template<class X, class Y, class C>
void init_enable_separated_from_this_(
    std::shared_ptr<typename separated_ptr<X>::data> const* ppx,
    Y const* py,
    enable_separated_from_this<C> const* pe
) {
    SEPARATE_DETAIL_DLOG("setting enable_separated_from_this for type " <<
            typeid(C).name());
    if(pe != 0) pe->accept_onwer_(ppx, const_cast<Y*>(py));
}

void init_enable_separated_from_this_(...) {
    SEPARATE_DETAIL_DLOG("not setting enable_separated_from_this");
}

template<class C, typename... Args>
separated_ptr<C> make_separated(Args&&... args) {
    separated_ptr<C> p;
    p.data_ = std::make_shared<typename separated_ptr<C>::data>(args...);
    init_enable_separated_from_this_(&p.data_, p.data_.get(), &p.data_->obj);
    return p;
}

#endif // #include guard

