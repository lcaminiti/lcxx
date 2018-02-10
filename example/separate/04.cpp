
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

std::atomic_bool done;
typedef std::packaged_task<void ()> task;
threadsafe_queue<task> tasks;

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

template<typename F>
void async(F f) { tasks.push(task(f)); }

template<typename F>
void sync(F f) {
    task t(f);
    std::future<void> r = t.get_future();
    tasks.push(std::move(t));
    r.get();
    std::cout << "passed future get" << std::endl;
}

int main() {
    done = false;
    std::thread th(work);
    
    async([] { std::cout << 1 << std::endl; });
    async([] { std::cout << 2 << std::endl; });
    
    sync([] { std::cout << 3 << std::endl; });
    
    done = true;
    async([] {}); // Last task, force out of wait_for_pop.
    th.join();
    
    return 0;
}

