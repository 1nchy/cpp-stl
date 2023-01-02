#ifndef _ASP_UNORDERED_MAP_HPP_
#define _ASP_UNORDERED_MAP_HPP_

#include <functional>

#include "basic_param.hpp"
#include "hash_table.hpp"

namespace asp {

template <typename _Key, typename _Tp,
 typename _Hash = std::hash<_Key>,
 typename _Alloc = std::allocator<std::pair<const _Key, _Tp>>
> class unordered_map;

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc>
class unordered_map {
    typedef unordered_map<_Key, _Tp, _Hash, _Alloc> self;
    typedef hash_table<_Key, std::pair<const _Key, _Tp>, _select_0x, _Hash, _Alloc> umap_ht;
    umap_ht _h;
public:
    typedef typename umap_ht::key_type key_type;
    typedef typename umap_ht::value_type value_type;
    typedef typename umap_ht::hasher hasher;
    typedef typename umap_ht::iterator iterator;
    typedef typename umap_ht::const_iterator const_iterator;

/// (de)constructor
    unordered_map() = default;
    unordered_map(const self& _x) : base(_x._h) {}
    virtual ~unordered_map() = default;
    
/// implement
    size_type size() const { return _h.size(); }
    bool empty() const { return _h.empty(); }
    iterator begin() { return _h.begin(); }
    iterator end() { return _h.end(); }
    const_iterator cbegin() const { return _h.cbegin(); }
    const_iterator cend() const { return _h.cend(); }
    
};

};

#endif // _ASP_UNORDERED_MAP_HPP_
