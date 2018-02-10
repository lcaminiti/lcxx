
#include <boost/any.hpp>
#include <future>

void f() {}
int g() { return -123; }

int main() {
    std::packaged_task<void ()> t(f);
    std::packaged_task<int ()> u(g);
    std::packaged_task<boost::any ()> v(g);
    std::packaged_task<boost::any ()> w(f);
    return 0;
}

