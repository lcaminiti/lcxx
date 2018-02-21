
#include <lcxx/separate.hpp>
#include <thread>
#include <string>
#include <iostream>

class hardware {
public:
    void display_enter_pin() {
        std::lock_guard<std::mutex> l(iom_);
        std::cout << "Please enter PIN (0-9)" << std::endl;
    }
    
    void display_pin_incorrect() {
        std::lock_guard<std::mutex> l(iom_);
        std::cout << "PIN incorrect" << std::endl;
    }
        
    void display_withdrawal_options() {
        std::lock_guard<std::mutex> l(iom_);
        std::cout
            << "Withdraw 50? (w)" << std::endl
            << "Display balance? (b)" << std::endl
            << "Cancel? (c)" << std::endl
        ;
    }

private:
    std::mutex iom_;
};

class bank;

class operation {
public:
    explicit operation(lcxx::separate<hardware>& hw, lcxx::separate<bank>& bk,
            unsigned pin_max_len) :
        hw_(hw), bk_(bk), pin_max_len_(pin_max_len)
    {}


    void card_inserted(std::string const& account) {
        lcxx::separate<hardware>::arg hw = hw_;
        account_ = account;
        pin_ = "";
        (hw->*&hardware::display_enter_pin)();
    }

    void digit_pressed(char digit);

    void pin_verified() {
        lcxx::separate<hardware>::arg hw = hw_;
        (hw->*&hardware::display_withdrawal_options)();
    }

    void pin_incorrect() {
        lcxx::separate<hardware>::arg hw = hw_;
        (hw->*&hardware::display_pin_incorrect)();
    }

private:
    lcxx::separate<hardware>& hw_;
    lcxx::separate<bank>& bk_;
    unsigned const pin_max_len_;
    std::string account_;
    std::string pin_;
};
    
// TODO: replace with separate_from_this
lcxx::separate<operation>* _op_;
    
class bank {
public:
    bank(unsigned balance, std::string const& pin) : balance_(balance),
            pin_(pin) {}

    void verify_pin(std::string const& account, std::string const& pin) {
        lcxx::separate<operation>::arg op = *_op_;
        if(pin == pin_) (op->*&operation::pin_verified)();
        else (op->*&operation::pin_incorrect)();
    }

private:
    unsigned balance_;
    std::string const pin_;
};
    
void operation::digit_pressed(char digit) {
    lcxx::separate<bank>::arg bk = bk_;
    pin_ += digit;
    if(pin_.length() == pin_max_len_) {
        (bk->*&bank::verify_pin)(account_, pin_);
    }
}
    
int main() {
    std::string secret_pin = "1234";
    unsigned const starting_balance = 199;
    
    lcxx::separate<hardware> hw;
    lcxx::separate<bank> bk(starting_balance, secret_pin);
    lcxx::separate<operation> op(hw, bk, secret_pin.length());
    _op_ = &op;

    bool quit_pressed = false;
    while(!quit_pressed) {
        char c = getchar();

        switch(state) {
            case init: {
                lcxx::separate<hardware>::arg h = hw;
                (h->*&hardware::display_enter_card)();
                state = insert_card;
            }
            case insert_card: {
                while(getchar() != 'i') {}
                lcxx::separate<operation>::arg o = op;
                (o->*&operation::card_inserted)("acc1234");
                state = enter_pin;
            }
            case enter_pin: {
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
                    case '9': (o->*&operation::digit_pressed)(c); break;
            }
        }
        
        
        
        
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
            case '9': (o->*&operation::digit_pressed)(c); break;
            //case 'b': op.sender().send(balance_pressed()); break;
            //case 'w': op.sender().send(withdraw_pressed(50)); break;
            //case 'c': op.sender().send(cancel_pressed()); break;
            case 'q': quit_pressed = true; break;
        }
    }

    return 0;
}

