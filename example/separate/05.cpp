
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
        std::lock_guard<std::mutex> l(mut_);
        data_.push(std::move(value));
        cond_.notify_one();
    }

    void wait_for_pop(T& value) {
        std::unique_lock<std::mutex> l(mut_);
        cond_.wait(l, [this] { return !data_.empty(); });
        value = std::move(data_.front());
        data_.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> l(mut_);
        return data_.empty();
    }

private:
    mutable std::mutex mut_;
    std::queue<T> data_;
    std::condition_variable cond_;
};

class worker_thread {
private:
    typedef std::packaged_task<boost::any ()> task;
public:
    worker_thread() : done(false) {
        th = std::thread(&worker_thread::work, this);
    }
    
    ~worker_thread() {
        done = true;
        async([] {}); // Last task, force out of wait_for_pop.
        th.join();
    }
    
    template<typename F>
    void async(F&& f) {
        // TODO: move this f...
        tasks.push(task([f] { f(); return 0; }));
    }

    template<typename R, typename F>
    R sync(F&& f) {
        task t(std::move(f));
        std::future<boost::any> r = t.get_future();
        tasks.push(std::move(t));
        boost::any v = r.get();
        std::cout << "passed future get" << std::endl;
        return boost::any_cast<R>(v);
    }

private:
    void work() {
        while(!done || !tasks.empty()) {
            task t;
            std::cout << "work waiting..." << std::endl;
            tasks.wait_for_pop(t);
            std::cout << "work start" << std::endl;
            t();
            std::cout << "work end" << std::endl;
        }
    }

    std::atomic_bool done;
    threadsafe_queue<task> tasks;
    std::thread th;
};

int main() {
    worker_thread th;

    th.async([] { std::cout << 1 << std::endl; });
    th.async([] { std::cout << 2 << std::endl; });
    
    int i = th.sync<int>([] { std::cout << 3 << std::endl; return 123; });
    std::cout << i << std::endl;
    
    return 0;
}

