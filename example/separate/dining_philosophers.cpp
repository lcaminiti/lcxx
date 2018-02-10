
#include <lxx/separate/separated_ptr.hpp>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>

class philosopher;

class spork {
public:
    spork(unsigned left, unsigned right);

    void pick(lxx::separate<philosopher>::controlled p);
    void put(lxx::separate<philosopher>::controlled p);

private:
    std::string id_;
};

class philosopher: public lxx::separate_from_this<philosopher> {
public:
    explicit philosopher(unsigned id, std::shared_ptr<lxx::separate<spork>> l,
            std::shared_ptr<lxx::separate<spork>> r, unsigned round_count) :
        id_(id), times_to_eat_(round_count), left_spork_(l), right_spork_(r)
    {}

    void live() {
        for(unsigned n = times_to_eat_; n > 0; --n) {
            LXX_ATOMIC_OUT(std::cout, "phil " << id_ << " round " << n <<
                    std::endl);
            think();
            eat(*left_spork_, *right_spork_);
        }
    }
    
private:
    void think() {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    void eat(lxx:separate<spork>::controlled l,
            lxx::separate<spork>::controlled r) {
        (l->*&spork::pick)(*separate_from_this);
        (r->*&spork::pick)(*separate_from_this);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        (l->*&spork::put)(*separate_from_this);
        (r->*&spork::put)(*separate_from_this);
    }

    unsigned id_, times_to_eat_;
    std::shared_ptr<lxx::separate<spork>> left_spork_, right_spork_;
};
    
spork::spork(unsigned left, unsigned right) {
    std::ostringstream oss;
    oss << left << "F" << right;
    id_ = oss.str();
}

void spork::pick(lxx::separate<philosopher>::controlled p) {
    //auto pid = (p->*&philosopher::id)();
    ATOMIC_OUT(std::cout, "spork " << id_ << " picked by philosopher" <<
            std::endl);
}

void spork::put(lxx::separate<philosopher>::controlled p) {
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

    std::vector<std::shared_ptr<lxx::separate<philosopher>>> v(
            philosopher_count);

    auto first_spork = std::make_shared<lxx::separate<spork>>(
            philosopher_count, 1);
    std::shared_ptr<separate<spork>> left_spork = first_spork, right_spork;
    for(unsigned i = 0; i < philosopher_count; ++i) {
        if(i < philosopher_count - 1) {
            right_spork = std::make_shared<lxx::separate<spork>>(i, i + 1);
        } else right_spork = first_spork;

        v[i] = std::make_shared<lxx::separate<philosopher>>(i, left_spork,
                right_spork, round_count);

        left_spork = right_spork;
    }
    
    std::for_each(v.begin(), v.end(),
        [] (std::shared_ptr<lxx::separate<philosopher>> sp) {
            lxx::separate<philosopher>::controlled p = sp;
            (p->*&philosopher::live)();
        }
    );
    return 0;
}

