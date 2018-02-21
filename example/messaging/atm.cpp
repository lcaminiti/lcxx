
#include <lcxx/messaging.hpp>
#include <thread>
#include <string>
#include <iostream>

struct withdraw {
    std::string account;
    unsigned amount;
    mutable lcxx::messaging::sender atm_queue;

    withdraw(std::string const& _account, unsigned _amount,
            lcxx::messaging::sender _atm_queue) :
        account(_account), amount(_amount), atm_queue(_atm_queue)
    {}
};

struct withdraw_ok {};

struct withdraw_denied {};

struct cancel_withdrawal {
    std::string account;
    unsigned amount;

    cancel_withdrawal(std::string const& _account, unsigned _amount) :
            account(_account), amount(_amount) {}
};

struct withdrawal_processed {
    std::string account;
    unsigned amount;

    withdrawal_processed(std::string const& _account, unsigned _amount) :
            account(_account), amount(_amount) {}
};

struct card_inserted {
    std::string account;

    explicit card_inserted(std::string const& _account) : account(_account) {}
};

struct digit_pressed {
    char digit;

    explicit digit_pressed(char _digit) : digit(_digit) {}
};

struct clear_last_pressed {};

struct eject_card {};

struct withdraw_pressed {
    unsigned amount;

    explicit withdraw_pressed(unsigned _amount) : amount(_amount) {}
};

struct cancel_pressed {};

struct issue_money {
    unsigned amount;

    explicit issue_money(unsigned _amount) : amount(_amount) {}
};

struct verify_pin {
    std::string account;
    std::string pin;
    mutable lcxx::messaging::sender atm_queue;

    verify_pin(std::string const& _account, std::string const& _pin,
            lcxx::messaging::sender _atm_queue) :
        account(_account), pin(_pin), atm_queue(_atm_queue)
    {}
};

struct pin_verified {};

struct pin_incorrect {};

struct display_enter_pin {};

struct display_enter_card {};

struct display_insufficient_funds {};

struct display_withdrawal_cancelled {};

struct display_pin_incorrect_message {};

struct display_withdrawal_options {};

struct get_balance {
    std::string account;
    mutable lcxx::messaging::sender atm_queue;

    get_balance(std::string const& _account,
            lcxx::messaging::sender _atm_queue) :
        account(_account), atm_queue(_atm_queue)
    {}
};

struct balance {
    unsigned amount;

    explicit balance(unsigned _amount) : amount(_amount) {}
};

struct display_balance {
    unsigned amount;

    explicit display_balance(unsigned _amount) : amount(_amount) {}
};

struct balance_pressed {};

class operation {
public:
    operation(lcxx::messaging::sender const& bank,
            lcxx::messaging::sender const& hardware) :
        bank_(bank), hardware_(hardware)
    {}

    operation(operation const&) = delete;
    operation& operator=(operation const&) = delete;

    void done() { sender().send(lcxx::messaging::close()); }

    void run() {
        state_ = &operation::wait_for_card;
        try { for(;;) (this->*state_)(); }
        catch(lcxx::messaging::close const&) {}
    }

    lcxx::messaging::sender sender() { return incoming_; }

private:
    void process_withdrawal() {
        incoming_.wait()
            .handle<withdraw_ok>([&] (auto const&) {
                hardware_.send(issue_money(withdrawal_amount_));
                bank_.send(withdrawal_processed(account_, withdrawal_amount_));
                state_ = &operation::done_processing;
            })
            .handle<withdraw_denied>([&] (auto const&) {
                hardware_.send(display_insufficient_funds());
                state_ = &operation::done_processing;
            })
            .handle<cancel_pressed>([&] (auto const&) {
                bank_.send(cancel_withdrawal(account_, withdrawal_amount_));
                hardware_.send(display_withdrawal_cancelled());
                state_ = &operation::done_processing;
            })
        ;
    }

    void process_balance() {
        incoming_.wait()
            .handle<balance>([&] (auto const& msg) {
                hardware_.send(display_balance(msg.amount));
                state_ = &operation::wait_for_action;
            })
            .handle<cancel_pressed>([&] (auto const&) {
                state_ = &operation::done_processing;
            })
        ;
    }

    void wait_for_action() {
        hardware_.send(display_withdrawal_options());
        incoming_.wait()
            .handle<withdraw_pressed>([&] (auto const& msg) {
                withdrawal_amount_ = msg.amount;
                bank_.send(withdraw(account_, msg.amount, incoming_));
                state_ = &operation::process_withdrawal;
            })
            .handle<balance_pressed>([&] (auto const&) {
                bank_.send(get_balance(account_, incoming_));
                state_ = &operation::process_balance;
            })
            .handle<cancel_pressed>([&] (auto const&) {
                state_ = &operation::done_processing;
            })
        ;
    }

    void wait_for_card() {
        hardware_.send(display_enter_card());
        incoming_.wait()
            .handle<card_inserted>([&] (auto const& msg) {
                account_ = msg.account;
                pin_ = "";
                hardware_.send(display_enter_pin());
                state_ = &operation::getting_pin;
            })
        ;
    }

    void done_processing() {
        hardware_.send(eject_card());
        state_ = &operation::wait_for_card;
    }

    void verifying_pin() {
        incoming_.wait()
            .handle<pin_verified>([&] (auto const& msg) {
                state_ = &operation::wait_for_action;
            })
            .handle<pin_incorrect>([&] (auto const& msg) {
                hardware_.send(display_pin_incorrect_message());
                state_ = &operation::done_processing;
            })
            .handle<cancel_pressed>([&] (auto const& msg) {
                state_ = &operation::done_processing;
            })
        ;
    }

    void getting_pin() {
        incoming_.wait()
            .handle<digit_pressed>([&] (auto const& msg) {
                unsigned const max_len = 4;
                pin_ += msg.digit;
                if(pin_.length() == max_len) {
                    bank_.send(verify_pin(account_, pin_, incoming_));
                    state_ = &operation::verifying_pin;
                }
            })
            .handle<clear_last_pressed>([&] (auto const& msg) {
                if(!pin_.empty()) pin_.pop_back();
            })
            .handle<cancel_pressed>([&] (auto const& msg) {
                state_ = &operation::done_processing;
            })
        ;
    }
    
    lcxx::messaging::receiver incoming_;
    lcxx::messaging::sender bank_;
    lcxx::messaging::sender hardware_;

    void (operation::* state_)();

    std::string account_;
    unsigned withdrawal_amount_;
    std::string pin_;
};

class hardware {
public:
    void done() { sender().send(lcxx::messaging::close()); }

    void run() {
        try {
            for(;;) {
                incoming_.wait()
                    .handle<issue_money>([&] (auto const& msg) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "Issuing " << msg.amount << std::endl;
                    })
                    .handle<display_insufficient_funds>([&] (auto const&) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "Insufficient funds" << std::endl;
                    })
                    .handle<display_enter_pin>([&] (auto const&) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "Please enter PIN (0-9)" << std::endl;
                    })
                    .handle<display_enter_card>([&] (auto const&) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "Please enter your card (i)" << std::endl;
                    })
                    .handle<display_balance>([&] (auto const& msg) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "Your account balance is " << msg.amount <<
                                std::endl;
                    })
                    .handle<display_withdrawal_options>([&] (auto const&) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout
                            << "Withdraw 50? (w)" << std::endl
                            << "Display balance? (b)" << std::endl
                            << "Cancel? (c)" << std::endl
                        ;
                    })
                    .handle<display_withdrawal_cancelled>([&] (auto const&) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "Withdrawal cancelled" << std::endl;
                    })
                    .handle<pin_incorrect>([&] (auto const&) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "PIN incorrect" << std::endl;
                    })
                    .handle<eject_card>([&] (auto const&) {
                        std::lock_guard<std::mutex> l(iom_);
                        std::cout << "Ejecting card" << std::endl;
                    })
                ;
            }
        } catch(lcxx::messaging::close const&) {}
    }

    lcxx::messaging::sender sender() { return incoming_; }

private:
    std::mutex iom_;
    lcxx::messaging::receiver incoming_;
};

class bank {
public:
    bank() : balance_(199) {}

    void done() { sender().send(lcxx::messaging::close()); }

    void run() {
        try {
            for(;;) {
                incoming_.wait()
                    .handle<verify_pin>([&] (auto const& msg) {
                        if(msg.pin == "1234") {
                            msg.atm_queue.send(pin_verified());
                        } else msg.atm_queue.send(pin_incorrect());
                    })
                    .handle<withdraw>([&] (auto const& msg) {
                        if(balance_ >= msg.amount) {
                            msg.atm_queue.send(withdraw_ok());
                            balance_ -= msg.amount;
                        } else msg.atm_queue.send(withdraw_denied());
                    })
                    .handle<get_balance>([&] (auto const& msg) {
                        msg.atm_queue.send(balance(balance_));
                    })
                    .handle<withdrawal_processed>([&] (auto const&) {})
                    .handle<cancel_withdrawal>([&] (auto const&) {})
                ;
            }
        } catch(lcxx::messaging::close const&) {}
    }

    lcxx::messaging::sender sender() { return incoming_; }

private:
    lcxx::messaging::receiver incoming_;
    unsigned balance_;
};

int main() {
    bank bk;
    hardware hw;
    operation op(bk.sender(), hw.sender());

    std::thread bk_td(&bank::run, &bk);
    std::thread hw_td(&hardware::run, &hw);
    std::thread op_td(&operation::run, &op);

    bool quit_pressed = false;
    while(!quit_pressed) {
        char c = getchar();
        switch(c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': op.sender().send(digit_pressed(c)); break;
        case 'b': op.sender().send(balance_pressed()); break;
        case 'w': op.sender().send(withdraw_pressed(50)); break;
        case 'c': op.sender().send(cancel_pressed()); break;
        case 'q': quit_pressed = true; break;
        case 'i': op.sender().send(card_inserted("acc1234")); break;
        }
    }

    bk.done();
    op.done();
    hw.done();

    op_td.join();
    bk_td.join();
    hw_td.join();

    return 0;
}

