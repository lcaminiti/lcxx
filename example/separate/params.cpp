
#include <separate/detail/type_traits.hpp>
#include <vector>
#include <type_traits>
#include <string>
#include <iostream>

template<typename F>
void check(F) {
    std::cout << separate_detail::has_handle<F>::value << std::endl;
}

struct x {
    int ok(int, float, std::string) { return -123; }
    void f() {}

    // Limitation... this will work because * is wrapped within another type...
    void v(std::vector<int*>) {}

    int p(int, float*, std::string) { return -123; }
    int fp(int, void(*)(), std::string) { return -123; }
    int mfp(int, void(x::*)(), std::string) { return -123; }
    int dmp(int x::*, std::string) { return -123; }
    int a(int, float[], std::string) { return -123; }
    int r(int, float&, std::string) { return -123; }

    void* rp(int, float, std::string) { return 0; }
    typedef void(*rfp_t)();
    rfp_t rfp(int, float, std::string) { return 0; }
    typedef void(x::*rmfp_t)();
    rmfp_t rmfp(int, float, std::string) { return 0; }
    float x::* rdmp(int, float, std::string) { return 0; }
    float** ra(int, float, std::string) { return 0; }
    float& rr(int, float, std::string) { return n_; }

private:
    float n_;
};

int main() {
    check(&x::ok);
    check(&x::f);
    check(&x::v);
    
    std::cout << "--" << std::endl;
    
    check(&x::p);
    check(&x::fp);
    check(&x::mfp);
    check(&x::dmp);
    check(&x::a);
    check(&x::r);
    
    std::cout << "--" << std::endl;
    
    check(&x::rp);
    check(&x::rfp);
    check(&x::rmfp);
    check(&x::rdmp);
    check(&x::ra);
    check(&x::rr);

    return 0;
}

