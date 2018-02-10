
#include <iostream>
#include <separate/separated_ptr.hpp>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

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

    void pick(separated_ptr<philosopher>::arg p);
    void put(separated_ptr<philosopher>::arg p);

private:
    std::string id_;
};

class philosopher : public enable_separated_from_this<philosopher> {
public:
    explicit philosopher(unsigned id, separated_ptr<spork> l,
            separated_ptr<spork> r, unsigned round_count) :
        id_(id), times_to_eat_(round_count), left_spork_(l), right_spork_(r)
    {}

    unsigned id() const { return id_; }

    void live(separated_ptr<philosopher> self) {
        for(unsigned n = times_to_eat_; n > 0; --n) {
            ATOMIC_OUT(std::cout, "phil " << id_ << " round " << n <<
                    std::endl);
            think();
            eat(self, left_spork_, right_spork_);
        }
    }
    
private:
    void think() {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    void eat(separated_ptr<philosopher> self,
            separated_ptr<spork>::arg l, separated_ptr<spork>::arg r) {
        (l->*&spork::pick)(separated_from_this());
        (r->*&spork::pick)(self);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        (l->*&spork::put)(self);
        (r->*&spork::put)(self);
    }

    unsigned id_, times_to_eat_;
    separated_ptr<spork> left_spork_, right_spork_;
};
    
spork::spork(unsigned left, unsigned right) {
    std::ostringstream oss;
    oss << left << "F" << right;
    id_ = oss.str();
}

void spork::pick(separated_ptr<philosopher>::arg p) {
    //auto pid = (p->*&philosopher::id)();
    ATOMIC_OUT(std::cout, "spork " << id_ << " picked by philosopher" <<
            std::endl);
}

void spork::put(separated_ptr<philosopher>::arg p) {
    //auto pid = 0;//(p->*&philosopher::id)();
    ATOMIC_OUT(std::cout, "spork " << id_ << " put back by philosopher" <<
            std::endl);
}

int main() {
    unsigned const philosopher_count = 5;
    unsigned const round_count = 30;

    std::vector<separated_ptr<philosopher> > v(philosopher_count);

    auto first_spork = make_separated<spork>(philosopher_count, 1);
    separated_ptr<spork> left_spork = first_spork, right_spork;
    for(unsigned i = 0; i < philosopher_count; ++i) {
        if(i < philosopher_count - 1) {
            right_spork = make_separated<spork>(i, i + 1);
        } else right_spork = first_spork;

        v[i] = make_separated<philosopher>(
                i, left_spork, right_spork, round_count);

        left_spork = right_spork;
    }
    
    std::for_each(v.begin(), v.end(),
        [] (separated_ptr<philosopher> p) {
            separated_ptr<philosopher>::arg a = p;
            (a->*&philosopher::live)(p);
        }
    );

    //TODO: how do I exit?
    std::this_thread::sleep_for(std::chrono::seconds(30));
    std::cout << "exiting..." << std::endl;
    return 0;
}

