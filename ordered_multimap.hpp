#ifndef _ASP_ORDERED_MULTI_MAP_HPP_
#define _ASP_ORDERED_MULTI_MAP_HPP_

#include <functional>

#include "basic_param.hpp"
#include "rb_tree.hpp"

namespace asp {

template <typename _Key, typename _Tp,
 typename _Compare = std::less<_Key>,
 typename _Alloc = std::allocator<std::pair<const _Key, _Tp>>
> class ordered_multimap;

template <typename _Key, typename _Tp, typename _Compare, typename _Alloc>
class ordered_multimap {
    typedef ordered_multimap<_Key, _Tp, _Compare, _Alloc> self;
    typedef rb_tree<_Key, std::pair<const _Key, _Tp>, _select_0x, false, _Compare, _Alloc> mmap_rbt;
    mmap_rbt _r;
public:
    typedef typename mmap_rbt::key_type key_type;
    typedef typename mmap_rbt::value_type value_type;
    typedef typename mmap_rbt::mapped_type mapped_type;
    typedef typename mmap_rbt::key_compare key_compare;
    typedef typename mmap_rbt::iterator iterator;
    typedef typename mmap_rbt::const_iterator const_iterator;
    typedef typename mmap_rbt::ireturn_type ireturn_type;
    typedef typename mmap_rbt::insert_status insert_status;
    typedef typename mmap_rbt::ext_iterator ext_iterator;
    typedef typename mmap_rbt::ext_key ext_key;
    typedef typename mmap_rbt::ext_value ext_value;

/// (de)constructor
    ordered_multimap() = default;
    ordered_multimap(const self& _x) : _r(_x._r) {}
    virtual ~ordered_multimap() = default;
    
/// implement
    size_type size() const { return _r.size(); }
    bool empty() const { return _r.empty(); }
    iterator begin() { return _r.begin(); }
    iterator end() { return _r.end(); }
    const_iterator cbegin() const { return _r.cbegin(); }
    const_iterator cend() const { return _r.cend(); }
    ireturn_type insert(const value_type& _v) { return _r.insert(_v); }
    ireturn_type set(const key_type& _k, const mapped_type& _m) { return _r.insert(value_type(_k, _m)); }
    size_type erase(const key_type& _k) { return _r.erase(_k); }
    size_type count(const key_type& _k) const { return _r.count(_k); }
    void clear() { _r.clear(); }
    iterator find(const key_type& _k) { return _r.find(_k); }
    const_iterator find(const key_type& _k) const { return _r.find(_k); }
#ifdef _CONTAINER_CHECK_
    int check() const { return _r.check(); }
#endif // _CONTAINER_CHECK_

/// output
    template <typename _K, typename _T, typename _C, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const ordered_multimap<_K, _T, _C, _A>& _um);
    // friend std::ostream& operator<<(std::ostream& os, const const_iterator& _i);
};

template <typename _Key, typename _Tp, typename _Comp, typename _Alloc> auto
operator<<(std::ostream& os, const ordered_multimap<_Key, _Tp, _Comp, _Alloc>& _um)
-> std::ostream& {
    os << _um._r;
    return os;
};

};

#endif // _ASP_ORDERED_MULTI_MAP_HPP_
