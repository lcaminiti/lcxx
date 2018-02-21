
#ifndef LCXX_PROPERTY_HPP_
#define LCXX_PROPERTY_HPP_

#include <utility>

namespace lcxx {

template<typename T>
class property {
public:
    typedef T value_type;
    typedef property<T> property_type;

    property() : value() {}
    /* implicit */ property(T const& v) : value(v) {}

    property(property const& other) : value(other.value) {}
    property const& operator=(property const& other) {
        set(other.value);
        return get();
    }

    property(property&& other) : value(std::move(other.value)) {}
    property const& operator=(property&& other) {
        set(std::move(other.value));
        return get();
    }
    
    virtual ~property() {}

    operator T const&() { return get(); }
    T const& operator=(T const& v) { set(v); return get(); }
    
    virtual T const& get() const { return value; }
    virtual void set(T const& v) { value = v; }

protected:
    T value;
};

} // namespace

#endif // #include guard

