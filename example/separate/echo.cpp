
#include <lxx/separate.hpp>
#include <string>
#include <iostream>

struct text {
    void set(std::string const& x) { x_ = x; }

    std::string get() const { return x_; }

private:
    std::string x_;
};

lxx::separate<text> stext;

struct in {
    void read_forever() {
        std::string x;
        while(true) {
            std::cin >> x;
            (stext->*&put)(x);
        }
    }
};

int main () {
    separate<in> sin;
    (sin->*&in::read_forever)();

    std::string cur, prev;
    while(x != "exit") {
        cur = (stext->*&text::get)();
        if(cur != prev) {
            std::cout << "Entered: " << cur << std::endl;
            prev = cur;
        }
    }
    return 0;
}

