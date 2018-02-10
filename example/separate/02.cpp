
#include <deque>
#include <future>
#include <mutex>
#include <thread>
#include <utility>
#include <iostream>

std::atomic_bool done;
std::mutex m;
std::deque<std::packaged_task<void ()> > tasks;

void work() {
    while(!done) {
        std::packaged_task<void ()> task;
        {
            std::lock_guard<std::mutex> l(m);
            if(tasks.empty()) continue;
            task = std::move(tasks.front());
            tasks.pop_front();
        }
        std::cout << "work start" << std::endl;
        task();
        std::cout << "work end" << std::endl;
    }
}

template<typename F>
std::future<void> push(F f) {
    std::packaged_task<void ()> task(f);
    std::future<void> r = task.get_future();
    std::lock_guard<std::mutex> l(m);
    tasks.push_back(std::move(task));
    return r;
}

int main() {
    done = false;
    std::thread th(work);
    push([] { std::cout << 1 << std::endl; });
    push([] { std::cout << 2 << std::endl; });
    auto x = push([] { std::cout << 3 << std::endl; });
    x.get();
    done = true;
    th.join();
    return 0;
}

