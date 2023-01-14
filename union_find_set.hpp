#ifndef _UNION_FIND_SET_H_
#define _UNION_FIND_SET_H_

#include "unordered_map.hpp"
#include "list.hpp"

namespace asp {

template <typename _Tp, typename _Hash = std::hash<_Tp>, typename _Alloc = std::allocator<_Tp>> class uf_set;

template <typename _Tp, typename _Hash, typename _Alloc> class uf_set {
    typedef typename hash_node<_Tp>::hash_code hash_code;

};

};

#endif