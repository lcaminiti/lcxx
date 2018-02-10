
#include <separate/separate.hpp>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>

#define ATOMIC_OUT(ostream, outputs) \
    { \
        std::ostringstream _out_; \
        _out_ << outputs; \
        ostream << _out_.str(); \
    }

class philosopher;

class spork {
public:
    spork(unsigned left, unsigned right);

    void pick(separate<philosopher>::arg p);
    void put(separate<philosopher>::arg p);

private:
    std::string id_;
};

class philosopher {
public:
    explicit philosopher(
        unsigned id,
        std::shared_ptr<separate<spork>> l,
        std::shared_ptr<separate<spork>> r,
        unsigned round_count
    ) :
        id_(id), times_to_eat_(round_count), left_spork_(l), right_spork_(r)
    {}

    unsigned id() const { return id_; }

    void live(std::shared_ptr<separate<philosopher>> self) {
        for(unsigned n = times_to_eat_; n > 0; --n) {
            ATOMIC_OUT(std::cout, "phil " << id_ << " round " << n <<
                    std::endl);
            think();
            eat(self, *left_spork_, *right_spork_);
        }
    }
    
private:
    void think() {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    void eat(std::shared_ptr<separate<philosopher>> p,
            separate<spork>::arg l, separate<spork>::arg r) {
        separate<philosopher>::arg x = *p;
        (l->*&spork::pick)(x);
        (r->*&spork::pick)(x);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        (l->*&spork::put)(x);
        (r->*&spork::put)(x);
    }

    unsigned id_, times_to_eat_;
    std::shared_ptr<separate<spork>> left_spork_, right_spork_;
};
    
spork::spork(unsigned left, unsigned right) {
    std::ostringstream oss;
    oss << left << "F" << right;
    id_ = oss.str();
}

void spork::pick(separate<philosopher>::arg p) {
    //auto pid = (p->*&philosopher::id)();
    ATOMIC_OUT(std::cout, "spork " << id_ << " picked by philosopher" <<
            std::endl);
}

void spork::put(separate<philosopher>::arg p) {
    //auto pid = 0;//(p->*&philosopher::id)();
    ATOMIC_OUT(std::cout, "spork " << id_ << " put back by philosopher" <<
            std::endl);
}

int main() {
    unsigned const philosopher_count = 5;
    unsigned const round_count = 30;
    std::cout << "dining philosophers" << std::endl;
    std::cout << philosopher_count << " philosophers, " << round_count <<
            " rounds" << std::endl;

    std::vector<std::shared_ptr<separate<philosopher>>> v(philosopher_count);

    auto first_spork = std::make_shared<separate<spork>>(philosopher_count, 1);
    std::shared_ptr<separate<spork> > left_spork = first_spork, right_spork;
    for(unsigned i = 0; i < philosopher_count; ++i) {
        if(i < philosopher_count - 1) {
            right_spork = std::make_shared<separate<spork>>(i, i + 1);
        } else right_spork = first_spork;

        v[i] = std::make_shared<separate<philosopher>>(
                i, left_spork, right_spork, round_count);

        left_spork = right_spork;
    }
    
    std::for_each(v.begin(), v.end(),
        [] (std::shared_ptr<separate<philosopher>> p) {
            separate<philosopher>::arg a = *p;
            (a->*&philosopher::live)(p);
        }
    );
    return 0;
}

