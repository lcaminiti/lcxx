
#include <boost/any.hpp>
#include <queue>
#include <future>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>
#include <iostream>

template<typename T>
class threadsafe_queue {
public:
    void push(T&& value) {
        std::lock_guard<std::mutex> l(mutex_);
        data_.push(std::move(value));
        cond_.notify_one();
    }

    void wait_for_pop(T& value) {
        std::unique_lock<std::mutex> l(mutex_);
        cond_.wait(l, [this] { return !data_.empty(); });
        value = std::move(data_.front());
        data_.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> l(mutex_);
        return data_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::queue<T> data_;
    std::condition_variable cond_;
};
    
    
template<typename R, typename F>
struct adapt_void {
    adapt_void(F&& f) : f_(std::move(f)) {}
    result<R> operator()() { return f_(); }
private:
    F f_;
};

template<typename R, typename F>
struct adapt_void {
    adapt_void(F&& f) : f_(std::move(f)) {}
    result<R> operator()() { return f_(); }
private:
    F f_;
};

class worker_thread {
private:
    typedef std::packaged_task<boost::any ()> task;
public:
    worker_thread() : done_(false) {
        th_ = std::thread(&worker_thread::work, this);
    }
    
    ~worker_thread() {
        done_ = true;
        async([] {}); // Last task, force out of wait_for_pop.
        th_.join();
    }
    
    template<typename F>
    void async(F&& f) {
        tasks_.push(task(std::move(adapt_void<F>(std::move(f)))));
    }

    template<typename R, typename F>
    R sync(F&& f) {
        task t(std::move(f));
        std::future<boost::any> r = t.get_future();
        tasks_.push(std::move(t));
        return boost::any_cast<R>(r.get());
    }

private:
    template<typename F>
    struct adapt_void {
        adapt_void(F&& f) : f_(std::move(f)) {}
        void* operator()() { f_(); return 0; }
    private:
        F f_;
    };

    void work() {
        while(!done_ || !tasks_.empty()) {
            task t;
            std::cout << "work waiting..." << std::endl;
            tasks_.wait_for_pop(t);
            std::cout << "work start" << std::endl;
            t();
            std::cout << "work end" << std::endl;
        }
    }

    std::atomic_bool done_;
    threadsafe_queue<task> tasks_;
    std::thread th_;
};

int main() {
    worker_thread th;

    th.async([] { std::cout << 1 << std::endl; });
    th.async([] { std::cout << 2 << std::endl; });
    
    int i = th.sync<int>([] { std::cout << 3 << std::endl; return 123; });
    std::cout << i << std::endl;
    
    return 0;
}

