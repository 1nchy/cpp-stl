#ifndef _ASP_TYPE_TRAITS_CPP_
#define _ASP_TYPE_TRAITS_CPP_

#include "type_traits.hpp"

#include "basic_param.hpp"

namespace asp {
/// main type
template <typename _Tp> struct is_array : public false_type {};
template <typename _Tp> struct is_array<_Tp[]> : public true_type {};
template <typename _Tp, size_type _N> struct is_array<_Tp[_N]> : public true_type {};

template <typename _Tp> struct is_pointer : public false_type {};
template <typename _Tp> struct is_pointer<_Tp*> : public true_type {};

template <typename _Tp> struct is_lvalue_reference : public false_type {};
template <typename _Tp> struct is_lvalue_reference<_Tp&> : public true_type {};

template <typename _Tp> struct is_rvalue_reference : public false_type {};
template <typename _Tp> struct is_rvalue_reference<_Tp&&> : public true_type {};

template <typename _Tp> struct is_function : public false_type {};
template <typename _Fn, typename... _Args> struct is_function<_Fn(_Args...)> : public true_type {};
template <typename _Fn, typename... _Args> struct is_function<_Fn(_Args......)> : public true_type {};


/// composite type
template <typename _Tp> struct is_reference : public false_type {};
template <typename _Tp> struct is_reference<_Tp&> : public true_type {};
template <typename _Tp> struct is_reference<_Tp&&> : public true_type {};


/// type attributes
template <typename _Tp> struct is_const : public false_type {};
template <typename _Tp> struct is_const<const _Tp> : public true_type {};

template <typename _Tp> struct is_volatile : public false_type {};
template <typename _Tp> struct is_volatile<volatile _Tp> : public true_type {};


/// type relationship
template <typename _Ty1, typename _Ty2> struct is_same : public false_type {};
template <typename _Ty> struct is_same<_Ty, _Ty> : public true_type {};

template <typename _From, typename _To> struct is_convertible;
template <typename _From, typename _To> struct is_convertible;


/// const-volatile modify
template <typename _Tp> struct add_const : public type_constant<const _Tp> {};

template <typename _Tp> struct add_volatile : public type_constant<volatile _Tp> {};

template <typename _Tp> struct add_cv : public type_constant<const volatile _Tp> {};

template <typename _Tp> struct remove_const : public type_constant<_Tp> {};
template <typename _Tp> struct remove_const<const _Tp> : public type_constant<_Tp> {};

template <typename _Tp> struct remove_volatile : public type_constant<_Tp> {};
template <typename _Tp> struct remove_volatile<volatile _Tp> : public type_constant<_Tp> {};

template <typename _Tp> struct remove_cv : public type_constant<_Tp> {};
template <typename _Tp> struct remove_cv<const _Tp> : public type_constant<_Tp> {};
template <typename _Tp> struct remove_cv<volatile _Tp> : public type_constant<_Tp> {};
template <typename _Tp> struct remove_cv<const volatile _Tp> : public type_constant<_Tp> {};


/// reference modify
template <typename _Tp> struct add_lvalue_reference : public type_constant<_Tp&> {};

template <typename _Tp> struct add_rvalue_reference : public type_constant<_Tp&&> {};

template <typename _Tp> struct remove_reference : public type_constant<_Tp> {};
template <typename _Tp> struct remove_reference<_Tp&> : public type_constant<_Tp> {};
template <typename _Tp> struct remove_reference<_Tp&&> : public type_constant<_Tp> {};


/// array modify
template <typename _Tp> struct remove_extent : public type_constant<_Tp> {};
template <typename _Tp> struct remove_extent<_Tp[]> : public type_constant<_Tp> {};
template <typename _Tp, size_type _N> struct remove_extent<_Tp[_N]> : public type_constant<_Tp> {};


/// pointer modify
template <typename _Tp> struct add_pointer : public type_constant<_Tp*> {};

template <typename _Tp> struct remove_pointer : public type_constant<_Tp> {};
template <typename _Tp> struct remove_pointer<_Tp*> : public type_constant<_Tp> {};


/// other conversion
template <bool _b, class _Ty1, class _Ty2> struct conditional : public type_constant<_Ty1> {};
template <class _Ty1, class _Ty2> struct conditional<false, _Ty1, _Ty2> : public type_constant<_Ty2> {};

// template <class _Tp> struct decay;

// template <bool _b, class _Tp = void> struct enable_if;
};

#endif
