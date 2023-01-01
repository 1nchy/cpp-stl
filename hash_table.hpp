#ifndef _ASP_HASH_TABLE_HPP_
#define _ASP_HASH_TABLE_HPP_

#include "node.hpp"
#include "hash_table_policy.hpp"
#include "type_traits.hpp"

#include <memory>

namespace asp {
    // maintain one hash_table
template <typename _Key, typename _Value, typename _ExtractKey, bool _Constant, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_node_iterator;
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_iterator;
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_const_iterator;
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> class hash_table;

/**
 * std::hash_table details:
 * _Args&... _args => _hash_node
 * _hash_node =(_M_extract)=> _key
 * _key =(_M_hash_code)=> _hash_code
 * {_key, _hash_code} =(_M_bucket_index)=> _bucket_index
 * 
 * unordered_map<_Key, _Value> = hash_table<_Key, std::pair<_Key, _Value>>
 * unordered_set<_Value> = hash_table<_Value, _Value>
 * _ExtractKey 是可调用类型，返回的是给 hashtable 计算哈希值的部分
 * 对于 map 和 set，这个部分都是 _Value 的第一个值，即 std::get<0>(...)
 * 需要牢记的点：在 hashtable 中，_Key 是从 _Value 算出来的，不是外界传进来的
*/

// _Key = decltype(std::get<0>(_Value))
template <typename _Key, typename _Value, typename _ExtractKey, bool _Constant, typename _Hash, typename _Alloc> struct hash_node_iterator {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, _ExtractKey, _Constant, _Hash, _Alloc> self;

    typedef hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc> _hash_table;

    typedef typename _hash_table::node_type node_type;
    typedef typename _hash_table::value_type value_type;
    typedef typename _hash_table::bucket_type bucket_type;
    typedef typename _hash_table::bucket_index bucket_index;

    typedef typename asp::conditional_t<_Constant, const node_type, node_type> _node_type;
    typedef typename asp::conditional_t<_Constant, const value_type, value_type> _value_type;

    _node_type* _cur = nullptr;
    bucket_index _bi = _hash_table::_s_illegal_index;
    const _hash_table* _ht = nullptr;

    hash_node_iterator() = default;
    hash_node_iterator(_node_type* _p, const _hash_table* _h) : _cur(_p), _ht(_h), _bi(_h->_M_bucket_find_index(_p)) {}
    hash_node_iterator(const bucket_index& _i, const _hash_table* _h) : _cur(_h->_M_bucket(_i)), _bi(_i), _ht(_h) {}
    hash_node_iterator(_node_type* _p, const bucket_index& _i, const _hash_table* _h) : _cur(_p), _ht(_h), _bi(_i) {}
    hash_node_iterator(const self& _s) : _cur(_s._cur), _bi(_s._bi), _ht(_s._ht) {}
    hash_node_iterator(self&& _s) : _cur(std::move(_s._cur)), _bi(std::move(_s._bi)), _ht(std::move(_s._ht)) {}
    virtual void _M_incr() {
        const _node_type* _old = _cur;
        _cur = _cur->_next;
        if (_cur == nullptr) {
            while (_cur == nullptr) {
                _ht->_M_next_bucket_index(_bi);
                if (_bi.first == -1) {
                    _cur = nullptr;
                    break;
                }
                _cur = _ht->_M_bucket(_bi);
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
    virtual operator bool() const {
        return _cur != nullptr && _ht != nullptr;
    }
    friend bool operator==(const self& _x, const self& _y) {
        return _x._cur == _y._cur && _x._ht == _y._ht;
    }
    friend bool operator!=(const self& _x, const self& _y) {
        return _x._cur != _y._cur || _x._ht != _y._ht;
    }
    template <typename _K, typename _V, typename _EK, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const hash_table<_K, _V, _EK, _H, _A>& _h);
    
    template <typename _K, typename _V, typename _EK, bool _C, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const hash_node_iterator<_K, _V, _EK, _C, _H, _A>& _h);

protected:
    bool _M_next_nullptr() const {
        return _cur == nullptr || _cur->_next == nullptr;
    }
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
 struct hash_iterator : public hash_node_iterator<_Key, _Value, _ExtractKey, false, _Hash, _Alloc> {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, _ExtractKey, false, _Hash, _Alloc> base;
    typedef hash_iterator<_Key, _Value, _ExtractKey, _Hash, _Alloc> self;
    typedef hash_const_iterator<_Key, _Value, _ExtractKey, _Hash, _Alloc> iterator;

    typedef typename base::_node_type node_type;
    typedef typename base::_hash_table _hash_table;
    typedef typename base::bucket_index bucket_index;

    hash_iterator() = default;
    hash_iterator(node_type* _p, const _hash_table* _h) : base(_p, _h) {}
    hash_iterator(const bucket_index& _i, const _hash_table* _h) : base(_i, _h) {}
    hash_iterator(node_type* _p, const bucket_index& _i, const _hash_table* _h) : base(_p, _i, _h) {}
    hash_iterator(const self& _s) : base(_s) {}
    hash_iterator(self&& _s) : base(std::move(_s)) {}

    iterator _const_cast() const {
        return *this;
    }
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
 struct hash_const_iterator : public hash_node_iterator<_Key, _Value, _ExtractKey, true, _Hash, _Alloc> {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, _ExtractKey, true, _Hash, _Alloc> base;
    typedef hash_const_iterator<_Key, _Value, _ExtractKey, _Hash, _Alloc> self;
    typedef hash_iterator<_Key, _Value, _ExtractKey, _Hash, _Alloc> iterator;

    typedef typename base::_node_type node_type;
    typedef typename base::_hash_table _hash_table;
    typedef typename base::bucket_index bucket_index;

    hash_const_iterator() = default;
    hash_const_iterator(const node_type* _p, const _hash_table* _h) : base(_p, _h) {}
    hash_const_iterator(const bucket_index& _i, const _hash_table* _h) : base(_i, _h) {}
    hash_const_iterator(const node_type* _p, const bucket_index& _i, const _hash_table* _h) : base(_p, _i, _h) {}
    hash_const_iterator(const self& _s) : base(_s) {}
    hash_const_iterator(self&& _s) : base(std::move(_s)) {}
    hash_const_iterator(const iterator& _it) : base(_it._cur, _it._bi, _it._ht) {}
    hash_const_iterator(iterator&& _it) : base(std::move(_it._cur), std::move(_it._bi), std::move(_it._ht)) {}

    iterator _const_cast() const {
        return iterator(const_cast<node_type*>(this->_cur), this->_bi, this->_ht);
    }
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
 class hash_table : public hash_table_alloc<_Value, _Alloc> {
public:
    typedef hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc> self;
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

    typedef hash_iterator<_Key, _Value, _ExtractKey, _Hash, _Alloc> iterator;
    typedef hash_const_iterator<_Key, _Value, _ExtractKey, _Hash, _Alloc> const_iterator;

    typedef rehash_policy::bucket_index bucket_index;
    static const bucket_index _s_illegal_index;

    bucket_type* _buckets = nullptr;
    size_type _bucket_count = 0;
    bucket_type* _rehash_buckets = nullptr;  // only used in rehash
    size_type _rehash_bucket_count = 0;
    node_type _before_begin;  // the node before @begin().
    size_type _element_count = 0;
    rehash_policy _rehash_policy;

    _ExtractKey _extract_key;

    template <typename _K, typename _V, typename _EK, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const hash_table<_K, _V, _EK, _H, _A>& _h);

public:
    hash_table();
    hash_table(const self& _ht);
    // self& operator=(const self& _r);
    virtual ~hash_table();
    template <typename _NodeGen> void _M_assign(const self& _ht, const _NodeGen&);

    iterator begin() { return iterator(_M_begin(), this); }
    const_iterator cbegin() const { return const_iterator(_M_begin(), this); }
    iterator end() { return iterator(nullptr, self::_s_illegal_index, this); }
    const_iterator cend() const { return const_iterator(nullptr, self::_s_illegal_index, this); }
    size_type size() const { return _element_count; }
    size_type bucket_count() const { return _rehash_policy._in_rehash ? _rehash_bucket_count : _bucket_count; }

    iterator find(const key_type& _k);
    const_iterator find(const key_type& _k) const;
    size_type count(const key_type& _k) const;
    void clear();

    void _M_deallocate_buckets() {
        base::_M_deallocate_buckets(_buckets, _bucket_count);
        base::_M_deallocate_buckets(_rehash_buckets, _rehash_bucket_count);
    }

    bucket_index _M_begin() const {
        bucket_index _i(0, 0);
        bucket_type _bkt = this->_M_bucket(_i);
        if (_bkt == nullptr) {
            this->_M_next_bucket_index(_i);
        }
        return _i;
    }
    hash_code _M_hash_code(const key_type& _k) const;
    bool _M_equals(const key_type& _k, hash_code _c, const node_type* _p) const;

    bool _M_valid_bucket_index(const bucket_index& _i) const;
    /**
     * @param: _p, a hash node which's not guaranteed to be in @hash_table
     * @return: the position where %_p would insert. always point to %_rehash_bucket when %_in_rehash = true.
    */
    bucket_index _M_bucket_insert_index(hash_code _c) const;
    bucket_index _M_bucket_insert_index(const node_type* _p) const {
        return _M_bucket_insert_index(_p->_hashcode);
    }
    /**
     * @param: _p, a hash node which's guaranteed to be in @hash_table
     * @return: the bucket index of %_p
    */
    bucket_index _M_bucket_find_index(const key_type& _k, hash_code _c) const;
    bucket_index _M_bucket_find_index(const node_type* _p) const {
        return _M_bucket_find_index(this->_extract_key(_p->val()), _p->_hash_code);
    }

    void _M_next_bucket_index(bucket_index& _i) const;
    bucket_type _M_bucket(const bucket_index& _i) const;
    bucket_type& _M_bucket_ref(const bucket_index& _i) const;
    // find node {_k, _c} in _bucket[_i]
    node_type* _M_find_node(const bucket_index& _i, const key_type& _k, hash_code _c) const;
    // find node whose %_next = {_k, _c} in _bucket[_i], only used in multi-insert and erase
    node_type* _M_find_before_node(const bucket_index& _i, const key_type& _k, hash_code _c) const;

    void _M_insert_bucket_begin(const bucket_index& _i, node_type* _n);
    void _M_remove_bucket_begin(const bucket_index& _i);

    iterator _M_insert_unique_node(const bucket_index& _i, hash_code _c, node_type* _n);
    iterator _M_insert_multi_node(const bucket_index& _i, hash_code _c, node_type* _n);
    // the node {_k, _c} must exist in @_M_bucket(_i), otherwise @fn would remove the head node in @_M_bucket(_i)
    iterator _M_erase_node(const bucket_index& _i, const key_type& _k, hash_code _c);
    /// implement
    iterator _M_insert_unique(const value_type& _v);
    iterator _M_insert_multi(const value_type& _v);
    iterator _M_erase(const key_type& _k);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::hash_table() {
    size_type _s = _prime_list[0];
    this->_buckets = this->_M_allocate_buckets(_s);
    this->_bucket_count = _s;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::hash_table(const self& _ht)
: base(_ht), _buckets(nullptr), _bucket_count(_ht._bucket_count)
, _rehash_buckets(nullptr), _rehash_bucket_count(_ht._rehash_bucket_count)
, _element_count(_ht._element_count), _rehash_policy(_ht._rehash_policy), _extract_key(_ht._extract_key) {
    _M_assign(_ht, [this](const node_type* _n) {
        return this->_M_allocate_node(*_n);
    });
};

// template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
// hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::operator=(const self& _r)
// -> self& {
//     if (&_r == this) {
//         return *this;
//     }

// };

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::~hash_table() {
    clear();
    this->_M_deallocate_buckets();
    _buckets = nullptr; _bucket_count = 0;
    _rehash_buckets = nullptr; _rehash_bucket_count = 0;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
template <typename _NodeGen> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::
_M_assign(const self& _ht, const _NodeGen& _gen) -> void {
    bucket_type* _t_buckets = nullptr;
    bucket_type* _t_rehash_buckets = nullptr;
    if (_buckets == nullptr) {
        _buckets = _t_buckets = this->_M_allocate_buckets(_ht._bucket_count);
        _rehash_buckets = _t_rehash_buckets = this->_M_allocate_buckets(_ht._rehash_bucket_count);
    }
    if (_element_count == 0) {
        return;
    }
    const_iterator _ht_n = _ht.cbegin();
    node_type* _prev = nullptr;
    for (; _ht_n != _ht.cend(); _ht_n._M_incr()) {
        node_type* _p = _gen(_ht_n._cur);
        if (this->_M_bucket(_ht_n._bi) == nullptr) {
            this->_M_bucket_ref(_ht_n._bi) = _p;
        }
        else if (_prev != nullptr) {
            _prev->_next = _p;
        }
        _prev = _p;
    }
};


template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_hash_code(const key_type& _k) const
-> hash_code {
    return hasher()(_k);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::
_M_equals(const key_type& _k, hash_code _c, const node_type* _p) const
-> bool {
    return _k == this->_extract_key(_p->val());
}

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_valid_bucket_index(const bucket_index& _i) const
-> bool {
    if (_i.first == 0) {
        return _i.second >= 0 && _i.second < this->_bucket_count;
    }
    if (_i.first == 1 && this->_rehash_policy._in_rehash) {
        return _i.second >= 0 && _i.second < this->_rehash_bucket_count;
    }
    return false;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_bucket_insert_index(hash_code _c) const
-> bucket_index {
    // always insert %_p into _rehash_bucket if %_in_rehash
    if (this->_rehash_policy._in_rehash) {
        bucket_index _i1 = std::make_pair(1, _c % this->_rehash_bucket_count);
        return _i1;
    }
    // always insert %_p into _bucket if not %_in_rehash
    bucket_index _i0 = std::make_pair(0, _c % this->_bucket_count);
    return _i0;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_bucket_find_index(const key_type& _k, hash_code _c) const
-> bucket_index {
    bucket_index _i0 = std::make_pair(0, _c % this->_bucket_count);
    for (node_type* _bkt = this->_buckets[_i0.second]; _bkt != nullptr; _bkt = _bkt->_next) {
        if (this->_M_equals(_k, _c, _bkt)) {
            return _i0;
        }
    }
    if (this->_rehash_policy._in_rehash) {
        bucket_index _i1 = std::make_pair(1, _c % this->_rehash_bucket_count);
        for (node_type* _bkt = this->_rehash_buckets[_i1.second]; _bkt != nullptr; _bkt = _bkt->_next) {
            if (this->_M_equals(_k, _c, _bkt)) {
                return _i1;
            }
        }
    }
    return _s_illegal_index;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_next_bucket_index(bucket_index& _i) const
-> void {
    if (!_M_valid_bucket_index(_i)) {
        _i.first = -1; _i.second = 0;
        return;
    }
    do {
        ++_i.second;
        if (_i.first == 0 && _i.second >= this->_bucket_count) {
            if (this->_rehash_policy._in_rehash) {
                _i.first = 1; _i.second = 0;
            }
            else {
                _i.first = -1; _i.second = 0;
                break;
            }
        }
        // if %_i.first == 1, then %_rehash_policy._in_rehash = true
        if (_i.first == 1 && _i.second >= this->_rehash_bucket_count) {
            _i.first = -1; _i.second = 0;
            break;
        }
    } while (this->_M_bucket(_i) == nullptr);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_bucket(const bucket_index& _i) const
-> bucket_type {
    if (!_M_valid_bucket_index(_i)) {
        return nullptr;
    }
    if (_i.first == 1 && _rehash_policy._in_rehash) {
        return this->_rehash_buckets[_i.second];
    }
    else if (_i.first == 0) {
        return this->_buckets[_i.second];
    }
    return nullptr;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_bucket_ref(const bucket_index& _i) const
-> bucket_type& {
    if (_i.first == 1) {
        return this->_rehash_buckets[_i.second];
    }
    return this->_buckets[_i.second];
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::
_M_find_node(const bucket_index& _i, const key_type& _k, hash_code _c) const
-> node_type* {
    node_type* _p = this->_M_bucket(_i);
    if (_p == nullptr) return nullptr;
    for (; _p != nullptr; _p = _p->_next) {
        if (this->_M_equals(_k, _c, _p)) {
            return _p;
        }
    }
    return nullptr;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::
_M_find_before_node(const bucket_index& _i, const key_type& _k, hash_code _c) const
-> node_type* {
    node_type* _head = this->_M_bucket(_i);
    if (_head == nullptr) return nullptr;
    for (node_type* _p = _head->_next; _p != nullptr; _p = _p->_next) {
        if (this->_M_equals(_k, _c, _p)) {
            return _head;
        }
        _head = _p;
    }
    return nullptr;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::
_M_insert_bucket_begin(const bucket_index& _i, node_type* _n)
-> void {
    bucket_type _hint = this->_M_bucket(_i);
    _n->_next = _hint;
    this->_M_bucket_ref(_i) = _n;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_remove_bucket_begin(const bucket_index& _i)
-> void {
    bucket_type _hint = this->_M_bucket(_i);
    if (_hint != nullptr) {
        this->_M_bucket_ref(_i) = _hint->_next;
    }
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::
_M_insert_unique_node(const bucket_index& _i, hash_code _c, node_type* _n)
-> iterator {
    _n->_hash_code = _c;
    this->_M_insert_bucket_begin(_i, _n);
    ++_element_count;
    return iterator(_n, _i, this);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::
_M_insert_multi_node(const bucket_index& _i, hash_code _c, node_type* _n)
-> iterator {
    _n->_hash_code = _c;
    const node_type* _hint = this->_M_bucket(_i);
    const key_type& _k = this->_extract_key(_n->val());
    node_type* _prev = ((_hint != nullptr && this->_M_equals(_k, _c, _hint)) ?
     this->_M_bucket(_i) : _M_find_before_node(_i, _k, _c));
    if (_prev != nullptr) {
        _n->_next = _prev->_next;
        _prev->_next = _n;
    }
    else {
        _M_insert_bucket_begin(_i, _n);
    }
    ++_element_count;
    return iterator(_n, _i, this);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_erase_node(const bucket_index& _i, const key_type& _k, hash_code _c)
-> iterator {
    node_type* _hint = this->_M_bucket(_i);
    if (_hint == nullptr) {
        return iterator(nullptr, this);
    }
    node_type* _prev = (this->_M_equals(_k, _c, _hint)) ?
     nullptr : _M_find_before_node(_i, _k, _c);
    node_type* _n = (_prev != nullptr ? _prev->_next : _hint);
    iterator _result(_n, _i, this);
    _result._M_incr();
    if (_prev != nullptr) {
        _n = _prev->_next;
        _prev->_next = _n->_next;
    }
    else {
        _M_remove_bucket_begin(_i);
    }
    this->_M_deallocate_node(_n);
    --_element_count;
    return _result;
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_insert_unique(const value_type& _v)
-> iterator {
    const key_type& _k = this->_extract_key(_v);
    hash_code _c = this->_M_hash_code(_k);
    bucket_index _i = this->_M_bucket_find_index(_k, _c);
    if (this->_M_valid_bucket_index(_i)) {
        node_type* _n = this->_M_bucket(_i);
        if (_n != nullptr) {
            return iterator(_n, _i, this);
        }
    }
    _i = this->_M_bucket_insert_index(_c);
    node_type* _n = this->_M_allocate_node(_v);
    return _M_insert_unique_node(_i, _c, _n);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_insert_multi(const value_type& _v)
-> iterator {
    const key_type& _k = this->_extract_key(_v);
    const hash_code _c = this->_M_hash_code(_k);
    const bucket_index _i = this->_M_bucket_insert_index(_c);
    node_type* _n = this->_M_allocate_node(_v);
    return _M_insert_multi_node(_i, _c, _n);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_M_erase(const key_type& _k)
-> iterator {
    const hash_code _c = this->_M_hash_code(_k);
    bucket_index _i = this->_M_bucket_find_index(_k, _c);
    if (!this->_M_valid_bucket_index(_i)) {
        return iterator(nullptr, _s_illegal_index, this);
    }
    return this->_M_erase_node(_i, _k, _c);
};

template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::find(const key_type& _k)
-> iterator {
    hash_code _c = this->_M_hash_code(_k);
    const bucket_index _i = this->_M_bucket_find_index(_k, _c);
    node_type* _p = this->_M_find_node(_i, _k, _c);
    return iterator(_p, _i, this);
};
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::find(const key_type& _k) const
-> const_iterator {
    hash_code _c = this->_M_hash_code(_k);
    const bucket_index _i = this->_M_bucket_find_index(_k, _c);
    node_type* _p = this->_M_find_node(_i, _k, _c);
    return const_iterator(_p, _i, this);
};
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::count(const key_type& _k) const
-> size_type {
    hash_code _c = this->_M_hash_code(_k);
    const bucket_index _i = this->_M_bucket_find_index(_k, _c);
    if (!this->_M_valid_bucket_index(_i)) {
        return 0;
    }
    node_type* _p = this->_M_find_node(_i, _k, _c);
    if (_p == nullptr) {
        return 0;
    }
    size_type _cnt = 0;
    for (; _p != nullptr; _p = _p->_next) {
        if (this->_M_equals(_k, _c, _p)) {
            ++_cnt;
        }
    }
    return _cnt;
};
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::clear()
-> void {
    for (bucket_index _i(0, 0); this->_M_valid_bucket_index(_i); this->_M_next_bucket_index(_i)) {
        for (node_type* _p = this->_M_bucket(_i); _p != nullptr;) {
            node_type* _tmp = _p->_next;
            this->_M_deallocate_node(_p);
            _p = _tmp;
        }
        this->_M_bucket_ref(_i) = nullptr;
    }
    this->_element_count = 0;
};


/// constexpr static const member
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc>
constexpr const rehash_policy::bucket_index
hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>::_s_illegal_index
 = bucket_index(-1, 0);


/// output stream
template <typename _Key, typename _Value, typename _ExtractKey, typename _Hash, typename _Alloc> auto
operator<<(std::ostream& os, const hash_table<_Key, _Value, _ExtractKey, _Hash, _Alloc>& _h)
-> std::ostream& {
    os << '[';
    for (auto p = _h.cbegin(); p != _h.cend();) {
        os << *p;
        bool _next_null = static_cast<hash_node_iterator<_Key, _Value, _ExtractKey, true, _Hash, _Alloc>>(p)._M_next_nullptr();
        if (++p != _h.cend()) {
            os << (_next_null ? "; " : ", ");
        }
    }
    os << ']';
    return os;
};

template <typename _Key, typename _Value, typename _ExtractKey, bool _Constant, typename _Hash, typename _Alloc> auto
operator<<(std::ostream& os, const hash_node_iterator<_Key, _Value, _ExtractKey, _Constant, _Hash, _Alloc>& _h)
-> std::ostream& {
    if (_h) {
        os << *_h;
    }
    else {
        os << "null";
    }
    return os;
}

};

#endif  // _ASP_HASH_TABLE_HPP_