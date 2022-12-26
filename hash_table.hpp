#ifndef _ASP_HASH_TABLE_HPP_
#define _ASP_HASH_TABLE_HPP_

#include "node.hpp"
#include "hash_table_policy.hpp"

#include <memory>

namespace asp {
template <typename _Key, typename _Value, typename _Hash, typename _Alloc = std::allocator<_Value>> class hash_table;

template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
 class hash_table : public hash_table_alloc<_Value, _Alloc> {
public:
    typedef hash_table_alloc<_Value, _Alloc> base;
    typedef hash_table_alloc<_Value, _Alloc> ht_alloc;
    typedef typename base::elt_allocator_type elt_allocator_type;
    typedef typename base::elt_alloc_traits elt_alloc_traits;
    typedef typename base::node_allocator_type node_allocator_type;
    typedef typename base::node_alloc_traits node_alloc_traits;
    typedef typename base::bucket_allocator_type bucket_allocator_type;
    typedef typename base::bucket_alloc_traits bucket_alloc_traits;

    typedef _Key key_type;
    typedef typename base::node_type node_type;
    typedef typename base::bucket_type bucket_type;
    typedef typename node_type::value_type value_type;
    typedef _Hash hasher;

    bucket_type* _buckets = &_single_bucket;
    size_type _bucket_count = 1;
    node_type _before_begin;
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
        return ht_alloc::_M_allocate_buckets(_n);
    }
    void _M_deallocate_buckets(bucket_type* _p, size_type _n) {
        if (_M_uses_single_bucket()) {
            return;
        }
        ht_alloc::_M_deallocate_buckets(_p, _n);
    }
    void _M_deallocate_buckets() {
        _M_deallocate_buckets(_buckets, _bucket_count);
    }
};
};

#endif  // _ASP_HASH_TABLE_HPP_