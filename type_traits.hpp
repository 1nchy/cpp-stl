#ifndef _ASP_TYPE_TRAITS_HPP_
#define _ASP_TYPE_TRAITS_HPP_

namespace asp {

template <typename _Tp, _Tp _v> struct integral_constant;

template <typename _Tp, _Tp _v> struct integral_constant {
    static constexpr _Tp value = _v;
    typedef _Tp value_type;
    typedef integral_constant<_Tp, _v> type;
    constexpr value_type operator()() const { return value; }
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

};

#endif