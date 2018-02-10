
#include <lxxx/singleton.hpp>
#include <cassert>

unsigned instances = 0;

struct a {
    a() { ++instances; }
    void f() {}
};

int main() {
    lcxx::singleton<a>().f();
    lcxx::singleton<a>().f();
    lcxx::singleton<a>().f();
    assert(instances == 1);
    return 0;
}

