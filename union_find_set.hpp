#ifndef _UNION_FIND_SET_H_
#define _UNION_FIND_SET_H_

#include "unordered_map.hpp"
#include "unordered_set.hpp"
#include "list.hpp"

namespace asp {

template <typename _Tp, typename _Hash = std::hash<_Tp>, typename _Alloc = std::allocator<_Tp>> class uf_set;
template <typename _Tp, typename _Hash = std::hash<_Tp>, typename _Alloc = std::allocator<_Tp>> struct _Bucket;


template <typename _Tp, typename _Hash, typename _Alloc> class uf_set {
    typedef typename hash_node<_Tp>::hash_code hash_code;
    typedef _Tp key_type;
    
    unordered_map<_Tp, std::shared_ptr<_Bucket<_Tp, _Hash, _Alloc>>, _Hash, _Alloc> _m_content;
    size_type _m_bucket_count = 0;


    bool sibling(const key_type& _x, const key_type& _y);
    size_type size() const { return _m_content.size(); }
    size_type sets_count() const { return _m_bucket_count; }
    bool insert(const key_type& _e);
    bool insert(const key_type& _s, const key_type& _e);
    size_type erase(const key_type& _e);
    void merge(const key_type& _x, const key_type& _y);
};



template <typename _Tp, typename _Hash, typename _Alloc> struct _Bucket {
    typedef typename hash_node<_Tp>::hash_code hash_code;
    
    unordered_set<_Tp, _Hash, _Alloc> _m_content;
    typedef typename unordered_set<_Tp, _Hash, _Alloc>::key_type key_type;
    typedef typename unordered_set<_Tp, _Hash, _Alloc>::value_type value_type;

    hash_code _M_bucket_id() const {
        if (_m_content.empty()) { return -1; }
        return _Hash()(_m_content.cbegin()->val());
    }
    size_type _M_erase(const key_type& _k) {
        return _m_content.erase(_k);
    }
    bool _M_insert(const value_type& _v) {
        return _m_content.insert(_v).second;
    }
    size_type size() const { return _m_content.size(); }
    bool empty() const { return _m_content.empty(); }
};

};

#endif