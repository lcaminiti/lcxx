
#include <lcxx/localvar.hpp>
#include <cassert>

extern lcxx::undeclare_localvar x;

void f(int x) {
    assert(lcxx::is_localvar_v<decltype(x)>);
    assert(LCXX_IS_LOCALVAR(x));
}

void g() {
    assert(!lcxx::is_localvar_v<decltype(x)>);
    assert(!LCXX_IS_LOCALVAR(x));
}

int main() {
    assert(!lcxx::is_localvar_v<decltype(x)>);
    assert(!LCXX_IS_LOCALVAR(x));
    
    int x = 123;
    
    assert(lcxx::is_localvar_v<decltype(x)>);
    assert(LCXX_IS_LOCALVAR(x));
    
    f(x);
    g();
    
    return 0;
}

