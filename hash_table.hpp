#ifndef _ASP_HASH_TABLE_HPP_
#define _ASP_HASH_TABLE_HPP_

#include "node.hpp"
#include "hash_table_policy.hpp"
#include "type_traits.hpp"

#include <memory>

namespace asp {
    // maintain one hash_table
template <typename _Key, typename _Value, bool _Constant, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_node_iterator;
template <typename _Key, typename _Value, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_iterator;
template <typename _Key, typename _Value, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_const_iterator;
template <typename _Key, typename _Value, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> class hash_table;

template <typename _Key, typename _Value, bool _Constant, typename _Hash, typename _Alloc> struct hash_node_iterator {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, _Constant, _Hash, _Alloc> self;

    typedef hash_table<_Key, _Value, _Hash, _Alloc> _hash_table;

    typedef typename _hash_table::node_type node_type;
    typedef typename _hash_table::value_type value_type;
    typedef typename _hash_table::bucket_type bucket_type;

    typedef typename asp::conditional_t<_Constant, const node_type, node_type> _node_type;
    typedef typename asp::conditional_t<_Constant, const value_type, value_type> _value_type;

    _node_type* _cur = nullptr;
    const _hash_table* _ht = nullptr;

    hash_node_iterator() = default;
    hash_node_iterator(const _node_type* _p, const _hash_table* _h) : _cur(_p), _ht(_h) {}
    hash_node_iterator(const self& _s) : _cur(_s._cur), _ht(_s._ht) {}
    hash_node_iterator(self&& _s) : _cur(std::move(_s._cur)), _ht(std::move(_s._ht)) {}
    void _M_incr() {
        const _node_type* _old = _cur;
        _cur = _cur->_next;
        if (_cur == nullptr) {
            auto _bkt = _ht->_M_bucket_index(_old->_hash_code);
            while (_cur == nullptr) {
                _ht->_M_next_bucket_index(_bkt);
                if (_bkt.first != -1) {
                    _cur = _ht->_M_bucket(_bkt);
                }
            }
        }
    }

    self _const_cast() const {
        return *this;
    }

    _value_type& operator*() const {
        return _cur->val(); // caution
    }
    _value_type* operator->() const {
        return _cur == nullptr ? nullptr : _cur->valptr();
    }
    self& operator++() {
        this->_M_incr();
        return *this;
    }
    self operator++(int) {
        self _ret(*this);
        this->_M_incr();
        return _ret;
    }
    self& operator=(const self& _s) {
        _cur = _s._cur;
    }
    self& operator=(self&& _s) {
        _cur = std::move(_s._cur);
    }
    friend bool operator==(const self& _x, const self& _y) {
        return _x._cur == _y._cur;
    }
    friend bool operator!=(const self& _x, const self& _y) {
        return _x._cur != _y._cur;
    }
};

template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
 struct hash_iterator : public hash_node_iterator<_Key, _Value, false, _Hash, _Alloc> {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, false, _Hash, _Alloc> base;
    typedef hash_iterator<_Key, _Value, _Hash, _Alloc> self;
    typedef hash_const_iterator<_Key, _Value, _Hash, _Alloc> iterator;

    typedef typename base::_node_type node_type;
    typedef typename base::_hash_table _hash_table;

    hash_iterator() = default;
    hash_iterator(node_type* _p, const _hash_table* _h) : base(_p, _h) {}
    hash_iterator(const self& _s) : base(_s) {}
    hash_iterator(self&& _s) : base(std::move(_s)) {}

    iterator _const_cast() const {
        return *this;
    }
};

template <typename _Key, typename _Value, typename _Hash, typename _Alloc>
 struct hash_const_iterator : public hash_node_iterator<_Key, _Value, true, _Hash, _Alloc> {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, true, _Hash, _Alloc> base;
    typedef hash_const_iterator<_Key, _Value, _Hash, _Alloc> self;
    typedef hash_iterator<_Key, _Value, _Hash, _Alloc> iterator;

    typedef typename base::_node_type node_type;
    typedef typename base::_hash_table _hash_table;

    hash_const_iterator() = default;
    hash_const_iterator(node_type* _p, const _hash_table* _h) : base(_p, _h) {}
    hash_const_iterator(const self& _s) : base(_s) {}
    hash_const_iterator(self&& _s) : base(std::move(_s)) {}

    iterator _const_cast() const {
        return iterator(const_cast<node_type*>(this->_cur), this->_ht);
    }
};

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
    typedef typename node_type::hash_code hash_code;
    typedef _Hash hasher;

    typedef hash_iterator<_Key, _Value, _Hash, _Alloc> iterator;
    typedef hash_const_iterator<_Key, _Value, _Hash, _Alloc> const_iterator;

    // (0, x) indicates _buckets[x]
    // (1, y) indicates _rehash_buckets[y]
    typedef std::pair<short, size_type> bucket_index;

    bucket_type* _buckets = nullptr;
    size_type _bucket_count = 0;
    bucket_type* _rehash_buckets = nullptr;  // only used in rehash
    size_type _rehash_bucket_count = 0;
    node_type _before_begin;  // the node before @begin().
    size_type _element_count = 0;
    rehash_policy _rehash_policy;

    iterator begin() { return iterator(_M_begin()); }
    const_iterator cbegin() const { return const_iterator(_M_begin()); }
    iterator end() { return iterator(nullptr); }
    const_iterator cend() const { return const_iterator(nullptr); }
    size_type size() const { return _element_count; }
    size_type bucket_count() const { return _bucket_count; }

    iterator find(const key_type& _k);
    size_type count(const key_type& _k);

    void _M_deallocate_buckets(bucket_type* _p, size_type _n) {
        ht_alloc::_M_deallocate_buckets(_p, _n);
    }
    void _M_deallocate_buckets() {
        _M_deallocate_buckets(_buckets, _bucket_count);
    }

    node_type* _M_begin() const {
        return static_cast<node_type*>(_before_begin._next);
    }
    bucket_index _M_bucket_index(node_type* _p) const;
    bucket_index _M_bucket_index(hash_code _c) const;
    bucket_index _M_next_bucket_index(const bucket_index& _i) const;
    void _M_next_bucket_index(bucket_index& _i) const;
    bucket_type _M_bucket(const bucket_index& _i) const;
    node_type* _M_find_node(size_type _bkt, const key_type& _k, hash_code _c) const;


    /// implement
};
};

#endif  // _ASP_HASH_TABLE_HPP_