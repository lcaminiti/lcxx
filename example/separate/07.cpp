
#include <iostream>
#include <separate.hpp>

struct x {
    void f() { std::cout << "f" << std::endl; }
    void g() { std::cout << "g" << std::endl; }
    int h() { std::cout << "h" << std::endl; return 123; }
};

int main() {
    separate<x> s;

    (s->*&x::f)();
    (s->*&x::g)();
    
    int i = (s->*&x::h)();
    std::cout << i << std::endl;
    
    return 0;
}

