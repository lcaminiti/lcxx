
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

private:
    mutable std::mutex mut_;
    std::queue<T> data_;
    std::condition_variable cond_;
};

std::atomic_bool done;
std::mutex m;
threadsafe_queue<std::packaged_task<void ()> > tasks;

void work() {
    while(!done) {
        std::packaged_task<void ()> task;
        //{
        //    std::lock_guard<std::mutex> l(m);
        //    if(tasks.empty()) continue;
        //    task = std::move(tasks.front());
        //    tasks.pop_front();
        //}
        std::cout << "work waiting..." << std::endl;
        tasks.wait_for_pop(task);
        std::cout << "work start" << std::endl;
        task();
        std::cout << "work end" << std::endl;
    }
}

template<typename F>
std::future<void> push(F f) {
    std::packaged_task<void ()> task(f);
    std::future<void> r = task.get_future();
    //std::lock_guard<std::mutex> l(m);
    //tasks.push_back(std::move(task));
    tasks.push(std::move(task));
    return r;
}

int main() {
    done = false;
    std::thread th(work);
    push([] { std::cout << 1 << std::endl; });
    push([] { std::cout << 2 << std::endl; });
    auto x = push([] { std::cout << 3 << std::endl; });
    x.get();
    std::cout << "passed future get" << std::endl;
    done = true;
    push([] {}); // Last task, force out of wait_for_pop.
    th.join();
    return 0;
}

