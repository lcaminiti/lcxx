
#ifndef SEPARATE_DETAIL_PROCESSOR_HPP_
#define SEPARATE_DETAIL_PROCESSOR_HPP_

#include <separate/detail/threadsafe_queue.hpp>
#include <separate/detail/debug.hpp>
#include <boost/any.hpp>
#include <atomic>
#include <future>
#include <thread>
#include <utility>

namespace separate_detail {

class processor {
private:
    typedef std::packaged_task<boost::any ()> task;

public:
    processor() : done_(false) {
        thread_ = std::thread(&processor::process, this);
    }
    
    ~processor() {
        done_ = true;
        wake();
        thread_.join();
    }
    
    template<typename F>
    void async(F&& f) { tasks_.push(task(adapt_void<F>(std::move(f)))); }

    template<typename R, typename F>
    R sync(F&& f) {
        task t(std::move(f));
        std::future<boost::any> r = t.get_future();
        tasks_.push(std::move(t));
        return boost::any_cast<R>(r.get());
    }

    // Push a blocking no-task to wait until it is processed.
    void syncronize() { sync<int>([] { return 0; }); }

private:
    template<typename F>
    class adapt_void {
    public:
        adapt_void(F&& f) : f_(std::move(f)) {}

        // TODO: uncomment followings
        //adapt_void(adapt_void const&) = delete;
        adapt_void& operator=(adapt_void const&) = delete;

        // Add unused result (void cannot be converted to boost::any).
        int operator()() { f_(); return 0; }

    private:
        F f_;
    };

    // Push a non-blocking no-op task forcing thread out of wait_for_pop.
    void wake() { async([] {}); }

    void process() {
        // TODO: Move object construction there.
        while(!done_ || !tasks_.empty()) {
            task t;
            SEPARATE_DETAIL_DLOG("thread waiting for work");
            tasks_.wait_for_pop(t);
            SEPARATE_DETAIL_DLOG("thread starting work");
            t();
            SEPARATE_DETAIL_DLOG("thread finished work");
        }
    }

    // Oder of these declarations matter for thread safety.
    std::atomic_bool done_;
    threadsafe_queue<task> tasks_;
    std::thread thread_;
};

} // namespace

#endif // #include guard

