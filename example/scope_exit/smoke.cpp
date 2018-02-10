
#include <lcxx/scope_exit.hpp>
#include <cassert>

int main() {
    bool exited = false;
    { lcxx::scope_exit([&] { exited = true; }); }
    assert(exited);
    return 0;
}

