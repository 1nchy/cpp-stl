#ifndef _ASP_UNORDERED_MAP_HPP_
#define _ASP_UNORDERED_MAP_HPP_

#include <functional>

#include "basic_param.hpp"

namespace asp {

template <typename _Key, typename _Tp,
 typename _Hash = std::hash<_Key>,
 typename _Pred = std::equal_to<_Key>
> class unordered_map;

template <typename _Key, typename _Tp,
 typename _Hash = std::hash<_Key>,
 typename _Pred = std::equal_to<_Key>
> class unordered_map {
public:
    typedef _Key key_type;
    typedef _Tp value_type;
    typedef _Tp* pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp& reference;
    typedef const _Tp& const_reference;
    typedef _Hash hasher;
};

};

#endif // _ASP_UNORDERED_MAP_HPP_
