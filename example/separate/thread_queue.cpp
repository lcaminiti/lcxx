
class thread_queue {
public:
    thread_queue() : done_(false), th_(&thread_queue::work, this) {}

    ~thread_queue() {
        done_ = true;
        th_.join();
    }

    template<typename F>
    void push(F f) { q_.push(std::function<void ()>(f)); }

private:
    void work() {
        while(!done_) {
            std::function<void ()> f;
            if(q_.try_pop(f)) f();
            else std::this_thread::yield();
        }
    }

    std::atomic_bool done_;
    thread_safe_queue<std::function<void ()> > q_;
    std::thread th_;
};

