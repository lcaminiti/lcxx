
#include <lxx/separate.hpp>
#include <string>
#include <iostream>

class hash_storage {
public:
    int hash_code() const { return code_; }

    void hash_code(lxx:separate<std::string>::arg s) {
        code_ = (s->*&std::string::size)();
    }

private:
    int code_;
};

void run(lxx::separate<has_storage>::arg h, lxx::separate<std::string>::arg s) {
    (h->*&has_storage::hash_code)(s);
    std::cout << h.hash_code() << std::endl;
}

int main() {
    lxx::seprate<has_storate> h;
    lxx::separate<std::string> s{"abc"};
    run(h, s);
    return 0;
}

