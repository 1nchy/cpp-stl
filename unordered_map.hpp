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
    typedef hash_table<_Key, std::pair<const _Key, _Tp>, _select_0x, true, _Hash, _Alloc> umap_ht;
    umap_ht _h;
public:
    typedef typename umap_ht::key_type key_type;
    typedef typename umap_ht::value_type value_type;
    typedef typename umap_ht::mapped_type mapped_type;
    typedef typename umap_ht::hasher hasher;
    typedef typename umap_ht::iterator iterator;
    typedef typename umap_ht::const_iterator const_iterator;
    typedef typename umap_ht::ireturn_type ireturn_type;
    typedef typename umap_ht::insert_status insert_status;
    typedef typename umap_ht::ext_iterator ext_iterator;
    typedef typename umap_ht::ext_key ext_key;
    typedef typename umap_ht::ext_value ext_value;

/// (de)constructor
    unordered_map() = default;
    unordered_map(const self& _x) : _h(_x._h) {}
    virtual ~unordered_map() = default;
    
/// implement
    size_type size() const { return _h.size(); }
    bool empty() const { return _h.empty(); }
    iterator begin() { return _h.begin(); }
    iterator end() { return _h.end(); }
    const_iterator cbegin() const { return _h.cbegin(); }
    const_iterator cend() const { return _h.cend(); }
    ireturn_type insert(const value_type& _v) { return _h.insert(_v); }
    ireturn_type set(const key_type& _k, const mapped_type& _m) { return _h.insert(value_type(_k, _m)); }
    size_type erase(const key_type& _k) { return _h.erase(_k); }
    size_type count(const key_type& _k) const { return _h.count(_k); }
    void clear() { _h.clear(); }
    iterator find(const key_type& _k) { return _h.find(_k); }
    const_iterator find(const key_type& _k) const { return _h.find(_k); }
#ifdef _CONTAINER_CHECK_
    int check() const { return _h.check(); }
#endif // _CONTAINER_CHECK_

/// output
    template <typename _K, typename _T, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const unordered_map<_K, _T, _H, _A>& _um);
    // friend std::ostream& operator<<(std::ostream& os, const const_iterator& _i);
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
operator<<(std::ostream& os, const unordered_map<_Key, _Tp, _Hash, _Alloc>& _um)
-> std::ostream& {
    os << _um._h;
    return os;
};

};

#endif // _ASP_UNORDERED_MAP_HPP_
