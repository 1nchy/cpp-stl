#ifndef _ASP_UNORDERED_MAP_HPP_
#define _ASP_UNORDERED_MAP_HPP_

#include <functional>

#include "basic_param.hpp"
#include "hash_table.hpp"

namespace asp {

template <typename _Key, typename _Value,
 typename _Hash = std::hash<_Key>,
 typename _Alloc = std::allocator<_Value>
> class unordered_map;

template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
class unordered_map : public hash_table<_Key, std::pair<_Key, _Value>, _select_0x, _Hash, _Alloc> {
public:
    
};

};

#endif // _ASP_UNORDERED_MAP_HPP_
