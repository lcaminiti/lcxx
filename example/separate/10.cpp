
#include <iostream>
#include <separate/separated_ptr.hpp>
#include <string>

struct echo {
    void say(std::string s) { said_ += s; }

    std::string said() const { return said_; }

private:
    std::string said_;
};

int main() {
    separated_ptr<echo> p = make_separated<echo>();
    [] (separated_ptr<echo>::arg a) {
        std::cout << "said \"" << (a->*&echo::said)() << "\"" << std::endl;
        (a->*&echo::say)("hello, world! ");
        std::cout << "said \"" << (a->*&echo::said)() << "\"" << std::endl;
        (a->*&echo::say)("hi 1! ");
        (a->*&echo::say)("hi 2! ");
        (a->*&echo::say)("hi 3! ");
        std::cout << "said \"" << (a->*&echo::said)() << "\"" << std::endl;
    }(p);
    return 0;
}

