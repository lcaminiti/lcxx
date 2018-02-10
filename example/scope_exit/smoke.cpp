
#include <lxx/scope_exit.hpp>
#include <cassert>

int main() {
    bool exited = false;
    { lxx::scope_exit([&] { exited = true; }); }
    assert(exited);
    return 0;
}

