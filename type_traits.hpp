#ifndef _ASP_TYPE_TRAITS_HPP_
#define _ASP_TYPE_TRAITS_HPP_

#include "basic_param.hpp"

namespace asp {

template <typename _Tp> struct type_constant;
template <typename _Tp, _Tp _v> struct integral_constant;

template <typename _Tp> struct type_constant {
    typedef _Tp type;
};
template <typename _Tp, _Tp _v> struct integral_constant {
    const static constexpr _Tp value = _v;
    typedef _Tp value_type;
    typedef integral_constant<_Tp, _v> type;
    constexpr operator value_type() const { return value; }
    constexpr value_type operator()() const { return value; }
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;
template <bool _b> using bool_t = typename integral_constant<bool, _b>::type;

/// main type
// template <typename _Tp> struct is_void;
// template <typename _Tp> struct is_null_pointer;
// template <typename _Tp> struct is_integral;
// template <typename _Tp> struct is_floating_point;
template <typename _Tp> struct is_array;
template <typename _Tp> struct is_pointer;
template <typename _Tp> struct is_lvalue_reference;
template <typename _Tp> struct is_rvalue_reference;
// template <typename _Tp> struct is_member_object_pointer;
// template <typename _Tp> struct is_member_function_pointer;
template <typename _Tp> struct is_function;

/// composite type
template <typename _Tp> struct is_reference;
// template <typename _Tp> struct is_arithmetic;
// template <typename _Tp> struct is_fundamental;
// template <typename _Tp> struct is_object;
// template <typename _Tp> struct is_scalar;
// template <typename _Tp> struct is_compound;
// template <typename _Tp> struct is_member_pointer;

/// type attributes
template <typename _Tp> struct is_const;
template <typename _Tp> struct is_volatile;
// template <typename _Tp, typename... _Args> struct is_constructible;
// template <typename _Tp> struct is_default_constructible;
// template <typename _Tp> struct is_copy_constructible;
// template <typename _Tp> struct is_move_constructible;
// template <typename _Tp> struct is_assignable;
// template <typename _Tp> struct is_copy_assignable;
// template <typename _Tp> struct is_move_assignable;

/// type relationship
template <typename _Ty1, typename _Ty2> struct is_same;
// template <typename _Ty1, typename _Ty2> struct is_base_of;
template <typename _From, typename _To> struct is_convertible;

/// const-volatile modify
template <typename _Tp> struct add_const;
template <typename _Tp> struct add_volatile;
template <typename _Tp> struct add_cv;
template <typename _Tp> struct remove_const;
template <typename _Tp> struct remove_volatile;
template <typename _Tp> struct remove_cv;

/// reference modify
template <typename _Tp> struct add_lvalue_reference;
template <typename _Tp> struct add_rvalue_reference;
template <typename _Tp> struct remove_reference;

/// array modify
// template <typename _Tp> struct remove_all_extents;
template <typename _Tp> struct remove_extent;

/// pointer modify
template <typename _Tp> struct add_pointer;
template <typename _Tp> struct remove_pointer;

/// other conversion
template <bool _b, class _Ty1, class _Ty2> struct conditional;
template <bool _b, class _Ty1, class _Ty2> 
using conditional_t = typename conditional<_b, _Ty1, _Ty2>::type;

template <class _Tp> struct decay;
template <class _Tp> using decay_t = typename decay<_Tp>::type;

template <bool _b, class _Tp = void> struct enable_if;
template <bool _b, class _Tp = void> using enable_if_t = typename enable_if<_b, _Tp>::type;

template <typename _Tp> struct is_integral;
template <typename _Tp> struct is_floating_point;

/**
 * @brief tuple traits
*/
template <size_type _i, typename _Tp> struct tuple_traits;
template <size_type _i, typename _Tp>
using tuple_traits_t = typename tuple_traits<_i, _Tp>::type;

/**
 * @brief function detection
 * @details 
 * class A {
 *   int f(char c);
 * };
 * int main(void) {
 *   _HAS_MEMBER_FUNC(f, 1);
 *   if (has_member_func_1<A, char>::value) {
 *     printf("exist!\n");
 *   }
 *   return 0;
 * }
*/
#define _HAS_MEMBER_FUNC(func, id) \
template <typename _T, typename... _Args>\
struct has_member_func_##id\
{\
private:\
    template <typename _U> static auto _M_check(int) -> decltype(std::declval<_U>().func(std::declval<_Args>()...), true_type());\
    template <typename _U> static auto _M_check(...) -> false_type;\
public:\
    enum {_value = is_same<decltype(_M_check<_T>(0)), true_type>::value};\
};

#define _HAS_FUNC(func, id) \
template <typename... _Args> \
struct has_func_##id { \
private: \
    static auto _M_check(int) -> decltype(func(std::declval<_Args>()...), true_type()); \
    static auto _M_check(...) -> false_type; \
public: \
    enum {_value = is_same<decltype(_M_check(0)), true_type>::value}; \
};

#define _HAS_MEMBER_OPERATOR(func, id) \
template <typename _T, typename... _Args> \
struct has_member_operator_##id { \
private: \
    template <typename _U> static auto _M_check(int) -> decltype(std::declval<_U>().operator func(std::declval<_Args>()...), true_type()); \
    template <typename _U> static auto _M_check(...) -> false_type; \
public: \
    enum {_value = is_same<decltype(_M_check<_T>(0)), true_type>::value}; \
};

#define _HAS_OPERATOR(func, id) \
template <typename... _Args> \
struct has_operator_##id { \
private: \
    static auto _M_check(int) -> decltype(operator func(std::declval<_Args>()...), true_type()); \
    static auto _M_check(...) -> false_type; \
public: \
    enum {_value = is_same<decltype(_M_check(0)), true_type>::value}; \
};

// template <typename _T, typename _R, typename... _Args> struct func;
// template <typename _T, typename _R, typename... _Args> struct func : public true_type {};
// template <typename _T, typename _R, typename... _Args> struct func : public false_type {};

};

#include "type_traits.cpp"
#endif