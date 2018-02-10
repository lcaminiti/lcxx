
#include <lxx/singleton.hpp>
#include <cassert>

unsigned instances = 0;

struct a {
    a() { ++instances; }
    void f() {}
};

int main() {
    lxx::singleton<a>().f();
    lxx::singleton<a>().f();
    lxx::singleton<a>().f();
    assert(instances == 1);
    return 0;
}

