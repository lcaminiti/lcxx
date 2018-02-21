
#include <lcxx/separate.hpp>
#include <boost/contract.hpp>
#include <iostream>

class inventory {
    friend class boost::contract::access;

    void invariant() const {
        BOOST_CONTRACT_ASSERT(item_ >= 0);
        if(!has_item()) BOOST_CONTRACT_ASSERT(item_ == 0);
    }

public:
    inventory() : item_(0) {
        boost::contract::check c = boost::contract::constructor(this)
            .postcondition([&] {
                BOOST_CONTRACT_ASSERT(!has_item());
            })
        ;
    }

    ~inventory() {
        boost::contract::check c = boost::contract::destructor(this);
    }

    int item() const {
        int result;
        boost::contract::check c = boost::contract::public_function(this)
            .precondition([&] {
                BOOST_CONTRACT_ASSERT(has_item());
            })
            .postcondition([&] {
                BOOST_CONTRACT_ASSERT(result > 0);
            })
        ;
        return result = item_;
    }

    bool has_item() const {
        boost::contract::check c = boost::contract::public_function(this);
        return item_ > 0;
    }

    void put(int value) {
        boost::contract::check c = boost::contract::public_function(this)
            .precondition([&] {
                BOOST_CONTRACT_ASSERT(value > 0);
                BOOST_CONTRACT_ASSERT(!has_item());
            })
            .postcondition([&] {
                BOOST_CONTRACT_ASSERT(has_item());
                BOOST_CONTRACT_ASSERT(item() == value);
            })
        ;
        item_ = value;
    }

    void remove() {
        boost::contract::check c = boost::contract::public_function(this)
            .precondition([&] {
                BOOST_CONTRACT_ASSERT(has_item());
            })
            .postcondition([&] {
                BOOST_CONTRACT_ASSERT(!has_item());
            })
        ;
        item_ = 0;
    }

private:
    int item_;
};

class producer : private boost::contract::constructor_precondition<producer> {
    friend class boost::contract::access;

public:
    producer(int count, lcxx::separate<inventory>& inven) :
        boost::contract::constructor_precondition<producer>([&] {
            BOOST_CONTRACT_ASSERT(count > 0);
        }),
        item_count(count), inven_(inven), value_(0)
    {
        std::cout << "Producer: Initializing to produce " << count <<
                " item(s)" << std::endl;
    }

    int const item_count;

    void live() {
        for(int i = 0; i < item_count; ++i) produce(inven_);
    }

private:
    void produce(lcxx::separate<inventory>::arg inven) {
        lcxx::separated::require(inven, [] (auto inven) {
            return !(inven->*&inventory::has_item)();
        });
        //boost::contract::check c = boost::contract::function()
        //    .precondition([&] {
        //        BOOST_CONTRACT_ASSERT(!(inven->*&inventory::has_item)());
        //    })
        //;
        ++value_;
        (inven->*&inventory::put)(value_);
        std::cout << "Producer: Produced " << value_ << std::endl;
    }

    lcxx::separate<inventory>& inven_;
    int value_;
};

class consumer : private boost::contract::constructor_precondition<consumer> {
    friend class boost::contract::access;

public:
    consumer(int count, lcxx::separate<inventory>& inven) :
        boost::contract::constructor_precondition<consumer>([&] {
            BOOST_CONTRACT_ASSERT(count > 0);
        }),
        item_count(count), inven_(inven)
    {
        std::cout << "Consumer: Initializing to consume " << count <<
                " item(s)" << std::endl;
    }

    int const item_count;

    void live() {
        for(int i = 0; i < item_count; ++i) {
            std::cout << "Consumer: Attempting to consume inventory..." <<
                    std::endl;
            consume(inven_);
        }
    }

private:
    void consume(lcxx::separate<inventory>::arg inven) {
        lcxx::separated::require(inven, [] (auto inven) {
            return (inven->*&inventory::has_item)();
        });
        //boost::contract::check c = boost::contract::function()
        //    .precondition([&] {
        //        BOOST_CONTRACT_ASSERT((inven->*&inventory::has_item)());
        //    })
        //;
        int item = (inven->*&inventory::item)();
        (inven->*&inventory::remove)();
        std::cout << "Consumer: Consumed " << item << std::endl;
    }

    lcxx::separate<inventory>& inven_;
};

void run(lcxx::separate<producer>::arg prod,
        lcxx::separate<consumer>::arg cons) {
    (prod->*&producer::live)();
    (cons->*&consumer::live)();
}

int main() {
    int const n = 1500;
    lcxx::separate<inventory> inven;
    lcxx::separate<producer> prod(n, inven);
    lcxx::separate<consumer> cons(n, inven);

    run(prod, cons);
    return 0;
}

