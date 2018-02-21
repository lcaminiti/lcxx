
#ifndef LCXX_MESSAGING_HPP_
#define LCXX_MESSAGING_HPP_

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace lcxx { namespace messaging {

namespace detail {
    struct message_base {
        virtual ~message_base() {}
    };

    template<typename Msg>
    struct wrapped_message : message_base {
        Msg message;

        explicit wrapped_message(Msg const& msg) : message(msg) {}
    };

    class queue {
    public:
        template<typename Msg>
        void push(Msg const& msg) {
            std::lock_guard<std::mutex> l(m_);
            q_.push(std::make_shared<wrapped_message<Msg>>(msg));
            c_.notify_all();
        }

        std::shared_ptr<message_base> wait_and_pop() {
            std::unique_lock<std::mutex> l(m_);
            c_.wait(l, [&] { return !q_.empty(); });
            auto r = q_.front();
            q_.pop();
            return r;
        }

    private:
        std::mutex m_;
        std::condition_variable c_;
        std::queue<std::shared_ptr<message_base>> q_;
    };
    
    struct void_dispatcher {
        bool dispatch(std::shared_ptr<detail::message_base> const&) {
            return false;
        }
    };
}
    
class close {};
    
template<typename PrevDispatcher = detail::void_dispatcher,
        typename Msg = close, typename Func = void (*)(Msg)>
class dispatcher {
public:
    explicit dispatcher(detail::queue* q) : q_(q), chained_(false) {}
    
    dispatcher(dispatcher const&) = delete;
    dispatcher& operator=(dispatcher const&) = delete;

    dispatcher(dispatcher&& other) : q_(other.q_), prev_(other.prev_),
            f_(std::move(other.f_)), chained_(other.chained_) {
        other.chained_ = true;
    }

    template<typename OtherMsg, typename OtherFunc>
    dispatcher<dispatcher, OtherMsg, OtherFunc> handle(OtherFunc&& f) {
        return dispatcher<dispatcher, OtherMsg, OtherFunc>(q_, this,
                std::forward<OtherFunc>(f));
    }

    ~dispatcher() noexcept(false) { if(!chained_) wait_and_dispatch(); }

private:
    template<typename OtherDispatcher, typename OtherMsg, typename OtherFunc>
    friend class dispatcher;
    
    dispatcher(detail::queue* q, PrevDispatcher* prev, Func&& f) : q_(q),
            prev_(prev), f_(std::forward<Func>(f)), chained_(false) {
        prev_->chained_ = true;
    }

    void wait_and_dispatch() {
        for(;;) {
            auto msg = q_->wait_and_pop();
            if(dispatch(msg)) break;
        }
    }

    bool dispatch(std::shared_ptr<detail::message_base> const& msg) {
        if(!prev_) {
            if(dynamic_cast<detail::wrapped_message<close>*>(msg.get())) {
                throw close();
            }
            return false;
        } else if(detail::wrapped_message<Msg>* w = dynamic_cast<
                detail::wrapped_message<Msg>*>(msg.get())) {
            f_(w->message);
            return true;
        } else return prev_->dispatch(msg);
    }

    detail::queue* q_;
    PrevDispatcher* prev_;
    Func f_;
    bool chained_;
};

class sender {
public:
    sender() : q_(nullptr) {}

    explicit sender(detail::queue* q) : q_(q) {}

    template<typename Msg>
    void send(Msg const& msg) { if(q_) q_->push(msg); }

private:
    detail::queue *q_; // Senders can be copied.
};

class receiver {
public:
    receiver() = default;

    receiver(receiver const&) = delete;
    receiver& operator=(receiver const&) = delete;

    operator sender() { return sender(&q_); }

    dispatcher<> wait() { return dispatcher<>(&q_); }
    
private:
    detail::queue q_; // Receiver cannot be copied.
};

} } // namespace

#endif // #include guard

