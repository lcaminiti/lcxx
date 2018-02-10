
#ifndef SEPARATE_DETAIL_THREADSAFE_QUEUE_HPP_
#define SEPARATE_DETAIL_THREADSAFE_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <condition_variable>

namespace separate_detail {

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

} // namespace

#endif // #include guard

