
#include <lcxx/separate.hpp>
#include <string>
#include <iostream>

struct str {
    explicit str(std::string const& s) : s_(s) {}

    unsigned size() const { return s_.size(); }
    
private:
    std::string s_;
};

class hash_storage {
public:
    int get_code() const { return code_; }

    void set_code(lcxx::separate<str>::arg s) {
        code_ = (s->*&str::size)();
    }

private:
    int code_;
};

void run(lcxx::separate<hash_storage>::arg h,
        lcxx::separate<str>::arg s) {
    (h->*&hash_storage::set_code)(s);
    std::cout << (h->*&hash_storage::get_code)() << std::endl;
}

int main() {
    lcxx::separate<hash_storage> h;
    lcxx::separate<str> s{"abc"};
    run(h, s);
    return 0;
}

