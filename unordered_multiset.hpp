#ifndef _ASP_UNORDERED_MULTISET_HPP_
#define _ASP_UNORDERED_MULTISET_HPP_

#include <functional>

#include "basic_param.hpp"
#include "hash_table.hpp"

namespace asp {

template <typename _Tp,
 typename _Hash = std::hash<_Tp>,
 typename _Alloc = std::allocator<_Tp>
> class unordered_multiset;

template <typename _Tp, typename _Hash, typename _Alloc>
class unordered_multiset {
    typedef unordered_multiset<_Tp, _Hash, _Alloc> self;
    typedef hash_table<_Tp, _Tp, _select_self, false, _Hash, _Alloc> umset_ht;
    umset_ht _h;
public:
    typedef typename umset_ht::key_type key_type;
    typedef typename umset_ht::value_type value_type;
    typedef typename umset_ht::mapped_type mapped_type;
    typedef typename umset_ht::hasher hasher;
    typedef typename umset_ht::iterator iterator;
    typedef typename umset_ht::const_iterator const_iterator;
    typedef typename umset_ht::ireturn_type ireturn_type;
    typedef typename umset_ht::_InsertStatus _InsertStatus;
    typedef typename umset_ht::_ExtractIterator _ExtractIterator;
    typedef typename umset_ht::_ExtractKey _ExtractKey;
    typedef typename umset_ht::_ExtractValue _ExtractValue;

/// (de)constructor
    unordered_multiset() = default;
    unordered_multiset(const self& _x) : _h(_x._h) {}
    virtual ~unordered_multiset() = default;
    
/// implement
    size_type size() const { return _h.size(); }
    bool empty() const { return _h.empty(); }
    iterator begin() { return _h.begin(); }
    iterator end() { return _h.end(); }
    const_iterator cbegin() const { return _h.cbegin(); }
    const_iterator cend() const { return _h.cend(); }
    ireturn_type insert(const value_type& _v) { return _h.insert(_v); }
    size_type erase(const key_type& _k) { return _h.erase(_k); }
    size_type count(const key_type& _k) const { return _h.count(_k); }
    void clear() { _h.clear(); }
    iterator find(const key_type& _k) { return _h.find(_k); }
    const_iterator find(const key_type& _k) const { return _h.find(_k); }

/// output
    template <typename _T, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const unordered_multiset<_T, _H, _A>& _um);
    // friend std::ostream& operator<<(std::ostream& os, const const_iterator& _i);
};

template <typename _Tp, typename _Hash, typename _Alloc> auto
operator<<(std::ostream& os, const unordered_multiset<_Tp, _Hash, _Alloc>& _um)
-> std::ostream& {
    os << _um._h;
    return os;
};

};

#endif // _ASP_UNORDERED_MULTISET_HPP_
