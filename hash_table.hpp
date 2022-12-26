#ifndef _ASP_HASH_TABLE_HPP_
#define _ASP_HASH_TABLE_HPP_

#include "node.hpp"
#include "hash_table_policy.hpp"

#include <memory>

namespace asp {
template <typename _Tp> struct hash_node;
template <typename _Key, typename _Value, typename _Hash, typename _Alloc = std::allocator<_Value>> class hash_table;

template <typename _Tp> struct hash_node : public node<_Tp> {
    typedef node<_Tp> base;
    typedef hash_node<_Tp> self;
    typedef _Tp value_type;
    typedef _Tp* pointer;

    hash_node() : base() {}
    hash_node(const hash_node& r) : base(r) {}
    
    self* _next = nullptr;
    size_type _hash_code;
};

template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
 class hash_table : public hash_table_alloc<rebind_alloc<hash_node<_Value>>> {
public:
    typedef hash_node<_Value> _Node;
    typedef _Node* bucket_type;
    typedef _Key key_type;
    typedef _Node::value_type value_type;
    typedef _Hash hasher;

    bucket_type* _buckets = &_single_bucket;
    size_type _bucket_count = 1;
    _Node _before_begin;
    size_type _element_count = 0;
    rehash_policy _rehash_policy;
    bucket_type _single_bucket = nullptr;

    bool _M_uses_single_bucket() const {
        return _buckets == &_single_bucket;
    }
    bucket_type* _M_allocate_buckets(size_type _n) {
        if (_n == 1) {
            _single_bucket = nullptr;
            return &_single_bucket;
        }
        return 
    }
};
};

#endif  // _ASP_HASH_TABLE_HPP_