
#include <lcxx/property.hpp>
#include <iostream>
#include <cassert>

struct a {
    lcxx::property<int> x;

    struct : lcxx::property<int> {
        using property<int>::property;
        virtual int const& get() const override {
            std::cout << "getting y: " << value << std::endl;
            return value;
        }
        virtual void set(int const& v) override {
            std::cout << "setting y: " << v << std::endl;
            value = v;
        }
    } y;

    a(int _x, int _y) : x(_x), y(_y) {}
};

int main() {
    a aa(123, 321);
    assert(aa.x == 123);
    assert(aa.y == 321);

    aa.x = -123;
    aa.y = -321;
    assert(aa.x == -123);
    assert(aa.y == -321);

    return 0;
}

