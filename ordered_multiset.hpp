#ifndef _ASP_ORDERED_MULTI_SET_HPP_
#define _ASP_ORDERED_MULTI_SET_HPP_

#include <functional>

#include "basic_param.hpp"
#include "rb_tree.hpp"

namespace asp {

template <typename _Tp,
 typename _Compare = std::less<_Tp>,
 typename _Alloc = std::allocator<_Tp>
> class ordered_multiset;

template <typename _Tp, typename _Compare, typename _Alloc>
class ordered_multiset {
    typedef ordered_multiset<_Tp, _Compare, _Alloc> self;
    typedef rb_tree<_Tp, _Tp, _select_self, false, _Compare, _Alloc> mset_rbt;
    mset_rbt _r;
public:
    typedef typename mset_rbt::key_type key_type;
    typedef typename mset_rbt::value_type value_type;
    typedef typename mset_rbt::mapped_type mapped_type;
    typedef typename mset_rbt::key_compare key_compare;
    typedef typename mset_rbt::iterator iterator;
    typedef typename mset_rbt::const_iterator const_iterator;
    typedef typename mset_rbt::ireturn_type ireturn_type;
    typedef typename mset_rbt::insert_status insert_status;
    typedef typename mset_rbt::ext_iterator ext_iterator;
    typedef typename mset_rbt::ext_key ext_key;
    typedef typename mset_rbt::ext_value ext_value;

/// (de)constructor
    ordered_multiset() = default;
    ordered_multiset(const self& _x) : _r(_x._r) {}
    virtual ~ordered_multiset() = default;
    
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
    template <typename _T, typename _C, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const ordered_multiset<_T, _C, _A>& _um);
    // friend std::ostream& operator<<(std::ostream& os, const const_iterator& _i);
};

template <typename _Tp, typename _Comp, typename _Alloc> auto
operator<<(std::ostream& os, const ordered_multiset<_Tp, _Comp, _Alloc>& _um)
-> std::ostream& {
    os << _um._r;
    return os;
};

};

#endif // _ASP_ORDERED_MULTI_SET_HPP_
