
#include <iostream>
#include <separate.hpp>

struct x {
    x(int, float, void*) {}

    // commands (async)

    void c0() { std::cout << "c0" << std::endl; }
    void cc0() const { std::cout << "cc0" << std::endl; }
    
    void c1(int a1) { std::cout << "c1 " << a1 << std::endl; }
    void cc1(int a1) const { std::cout << "cc1 " << a1 << std::endl; }

    // queries (sync)

    int q0() { std::cout << "q0" << std::endl; return 0; }
    int cq0() const { std::cout << "cq0" << std::endl; return 0; }
    
    int q1(int a1) { std::cout << "q0 " << a1 << std::endl; return 0; }
    int cq1(int a1) const { std::cout << "cq0 " << a1 << std::endl; return 0; }

    // errors
    int const& qr() const { return i_; }
    int const* const qp() const { return &i_; }

private:
    int i_;
};

int main() {
    separate<x, separated::allow_handles> s(123, 1.23, nullptr);
    separate<x> const cs(123, 1.23, nullptr);

    SEPARATED(s, c0)();

    (s->*&x::c0)();
    (s->*&x::cc0)();
    
    //(cs->*&x::c0)(); // ok, const error
    (cs->*&x::cc0)();
    
    (s->*&x::q0)();
    (s->*&x::cq0)();
    
    //(cs->*&x::q0)(); // ok, const error
    (cs->*&x::cq0)();

    (s->*&x::c1)(123);
    (s->*&x::cc1)(123);
    
    //(cs->*&x::c1)(123); // ok, const error
    (cs->*&x::cc1)(123);
    
    (s->*&x::q1)(321);
    (s->*&x::cq1)(321);
    
    //(cs->*&x::q1)(321); // ok, const error
    (cs->*&x::cq1)(321);

    //(s->*&x::qr)(); // ok, return an handle
    (s->*&x::qp)(); // ok, return an handle

    return 0;
}

