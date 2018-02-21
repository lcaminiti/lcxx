
#ifndef LCXX_SEPARATE_SEPARATE_HPP_
#define LCXX_SEPARATE_SEPARATE_HPP_

#include <lcxx/separate/detail/call.hpp>
#include <lcxx/separate/detail/processor.hpp>
#include <lcxx/separate/detail/type_traits.hpp>
#include <lcxx/debug.hpp>
#include <mutex>
#include <type_traits>
#include <memory>
#include <typeinfo>
#include <cassert>

namespace lcxx {

template<class C>
class separate {
public:
    typedef C class_type;
    
    class movable_arg {
    private:
        template<class QualifiedC, typename FP>
        struct call {
            typedef separate_detail::call<
                separate_detail::is_command<FP>::value,
                QualifiedC,
                FP
            > type;
        };
        
    public:
        /* implicit */ movable_arg(separate& s) : lock_(s.mutex_),
                obj_(s.obj_), proc_(s.proc_) {}

        movable_arg(movable_arg const& other) = delete;
        movable_arg& operator=(movable_arg const&) = delete;

        template<typename FP>
        typename call<C, FP>::type operator->*(FP fp) {
            LCXX_DLOG("operator->* mutable");
            return typename call<C, FP>::type(proc_, obj_, fp);
        }
        
    private:
        std::lock_guard<std::mutex> lock_;
        C& obj_;
        separate_detail::processor& proc_;
    };
    typedef movable_arg&& arg;

    template<typename... Args>
    explicit separate(Args&&... args) : obj_(args...) {}

    separate(separate const&) = delete;
    separate& operator=(separate const&) = delete;
    
private:
    std::mutex mutex_;
    C obj_;
    separate_detail::processor proc_;
};

} // namespace

#endif // #include guard

