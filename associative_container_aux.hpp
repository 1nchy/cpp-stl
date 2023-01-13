#ifndef _ASP_ASSOCIATIVE_CONTAINER_AUX_HPP_
#define _ASP_ASSOCIATIVE_CONTAINER_AUX_HPP_

#include "type_traits.hpp"

#include <utility>

namespace asp {

namespace __details__ {
template <typename _Tp> struct pair_tail;
template <typename _Tp> struct pair_tail {
    typedef _Tp type;
};
template <typename _Head, typename _Tail> struct pair_tail<std::pair<_Head, _Tail>> {
    typedef _Tail type;
};
template <typename _Tp> using pair_tail_t = typename pair_tail<_Tp>::type;

template <typename _Tp> struct pair_head;
template <typename _Tp> struct pair_head {
    typedef _Tp type;
};
template <typename _Head, typename _Tail> struct pair_head<std::pair<_Head, _Tail>> {
    typedef _Head type;
};
template <typename _Tp> using pair_head_t = typename pair_head<_Tp>::type;
};

struct _select_self {
    template <typename _Tp> auto operator()(_Tp&& _x) const {
        return std::forward<_Tp>(_x);
    }
};

struct _select_0x {
    template <typename _Tp> auto operator()(_Tp&& _x) const {
        return std::get<0>(std::forward<_Tp>(_x));
    }
};

struct _select_1x {
    template <typename _Tp> auto operator()(_Tp&& _x) const {
        return std::get<1>(std::forward<_Tp>(_x));
    }
};

/**
 * @brief type traits for associative container
*/
namespace asso_container {
template <typename _Value, bool _UniqueKey> struct type_traits;
template <bool _b> struct bool_return;

template <typename _Value, bool _UniqueKey> struct type_traits {
    typedef _Value value_type;
    typedef __details__::pair_tail_t<_Value> mapped_type;
    typedef asp::decay_t<__details__::pair_head_t<_Value>> key_type;
    typedef asp::conditional_t<asp::is_same<key_type, value_type>::value, true_type, false_type> _kv_integration;
    typedef asp::conditional_t<_kv_integration::value, _select_self, _select_0x> ext_key;
    typedef asp::conditional_t<_kv_integration::value, _select_self, _select_1x> ext_value;
    typedef asp::conditional_t<_UniqueKey, _select_1x, bool_return<true>> insert_status;
    typedef asp::conditional_t<_UniqueKey, _select_0x, _select_self> ext_iterator;
};

template <bool _b> struct bool_return {
    template <typename _Tp> bool operator()(_Tp&& _x) const {
        return _b;
    };
};

};

};

#endif // _ASP_ASSOCIATIVE_CONTAINER_AUX_HPP_