
#ifndef LCXX_SEPARATE_DETAIL_CALL_HPP_
#define LCXX_SEPARATE_DETAIL_CALL_HPP_

#include <lcxx/separate/detail/processor.hpp>
#include <lcxx/separate/detail/type_traits.hpp>
#include <lcxx/debug.hpp>
#include <functional>

// TODO: For handles, the following binds need to apply ref and cref to Args... how do I do that?

// TODO: Test we are not copying function arguments and results more than strictly necessary (they should be copied only once into bind and be moved around after that).

namespace lcxx { namespace separate_detail {

template<bool IsCommand, class C, typename FP>
class call { // Command (async).
public:
    typedef void result_type;

    call(processor& proc, C& obj, FP fp) : proc_(proc), obj_(obj), fp_(fp) {}

    template<typename... Args>
    void operator()(Args&&... args) const {
        LCXX_DLOG("called command");
        proc_.async(std::bind(fp_, &obj_, args...));
    }
    
private:
    processor& proc_;
    C& obj_;
    FP fp_;
};

template<class C, typename FP>
class call<false, C, FP> { // Query (sync).
public:
    typedef typename boost::function_types::result_type<FP>::type
            result_type;

    call(processor& proc, C& obj, FP fp) : proc_(proc), obj_(obj), fp_(fp) {}

    template<typename... Args>
    result_type operator()(Args&&... args) const {
        LCXX_DLOG("called query");
        return proc_.template sync<result_type>(std::bind(fp_, &obj_, args...));
    }
    
private:
    processor& proc_;
    C& obj_;
    FP fp_;
};

} } // namespace

#endif // #include guard

