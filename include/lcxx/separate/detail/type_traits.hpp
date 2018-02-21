
#ifndef LCXX_SEPARATE_DETAIL_TYPE_TRAITS_HPP_
#define LCXX_SEPARATE_DETAIL_TYPE_TRAITS_HPP_

#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/placeholders.hpp>
#include <type_traits>
#include <cstddef>

namespace lcxx { namespace separate_detail {

namespace type_traits_ {
    template<typename Seq, typename Pred>
    struct contains_if_ :
        std::integral_constant<bool,
            !std::is_same<
                typename boost::mpl::find_if<Seq, Pred>::type,
                typename boost::mpl::end<Seq>::type
            >::value
        >
    {};

    template<typename F, typename Pred>
    struct contains_if :
        contains_if_<
            typename boost::mpl::push_front<
                // Pop reference to member function's class type (parameter 0th).
                typename boost::mpl::pop_front<boost::function_types::
                        parameter_types<F> >::type,
                // Include result type.
                typename boost::function_types::result_type<F>::type
            >::type,
            Pred
        >
    {};

    // TODO: Add pointers to member functions and data members
    template<typename T> struct remove_handle { typedef T type; };
    template<typename T> struct remove_handle<T&> : remove_handle<T> {};
    //TODO:template<typename T> struct remove_handle<T&&> : remove_handle<T> {};
    template<typename T> struct remove_handle<T*> : remove_handle<T> {};
    template<typename T> struct remove_handle<T* const> : remove_handle<T> {};
    template<typename T> struct remove_handle<T[]> : remove_handle<T> {};
    template<typename T, std::size_t N> struct remove_handle<T[N]> :
            remove_handle<T> {};
}

template<typename F>
struct is_const :
    boost::function_types::is_member_function_pointer<F,
            boost::function_types::const_qualified>
{};

// An handle is a pointer (of any kind: pointers to values, pointers to
// references, pointers to pointers, arrays, function pointers, member function
// pointers, pointers to data members, etc.) or a reference (regardless of
// their possible cv-qualifiers).
// TODO: add is_data_member_pointer (probably need to program that... not sure if it's part of STL or Boost already...).
template<typename T>
struct is_handle :
    std::integral_constant<bool,
        // Also cv-qualified (before after pointer, before ref, etc.)
        // Also array, function pointer, but not member function pointer.
        std::is_pointer<T>::value ||
        std::is_member_function_pointer<T>::value ||
        // TODO: Both l- and r- value reference.
        std::is_lvalue_reference<T>::value
    >
{};

template<typename F>
struct has_handle : type_traits_::contains_if<F, is_handle<boost::mpl::_1> > {};

template<typename T>
struct is_out :
    std::integral_constant<bool,
        is_handle<T>::value &&
        !std::is_const<typename type_traits_::remove_handle<T>::type>::value
    >
{};

template<typename F>
struct is_command :
    std::integral_constant<bool,
        std::is_same<void, typename boost::function_types::result_type<F>::
                type>::value &&
        !type_traits_::contains_if<F, is_out<boost::mpl::_1> >::value
    >
{};

} } // namespace

#endif // #include guard

