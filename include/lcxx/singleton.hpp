
#ifndef LCXX_SINGLETON_HPP_
#define LCXX_SINGLETON_HPP_

namespace lcxx {

template<class C>
C& singleton() {
    static C obj; // Init only once, plus always thread-safe in C++11.
    return obj;
}

} // namespace

#endif // #include guard

