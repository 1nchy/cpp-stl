#ifndef _ASP_HASH_TABLE_HPP_
#define _ASP_HASH_TABLE_HPP_

#include "node.hpp"
#include "hash_table_policy.hpp"
#include "type_traits.hpp"

#include "associative_container_aux.hpp"

#include "basic_io.hpp"

#define _HASH_TABLE_CHECK_
#ifdef _HASH_TABLE_CHECK_
#include <unordered_set>
#endif // _HASH_TABLE_CHECK_

// sacrificing the efficieny to promise that the same key-values are stored adjacently.
#define _HASH_TABLE_ADJACENT_SAME_VALUE_
// #undef _HASH_TABLE_ADJACENT_SAME_VALUE_

#include <cassert>
#include <memory>

namespace asp {
    // maintain one hash_table
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, bool _Constant, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_node_iterator;
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_iterator;
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct hash_const_iterator;
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> class hash_table;

/**
 * @brief std::hash_table
 * @details
 * _Args&... _args => _hash_node
 * _hash_node =(_M_extract)=> _key
 * _key =(_M_hash_code)=> _hash_code
 * {_key, _hash_code} =(_M_bucket_index)=> _bucket_index
 * 
 * unordered_map<_Key, _Value> = hash_table<_Key, std::pair<const _Key, _Value>>
 * unordered_set<_Value> = hash_table<_Value, _Value>
 * _ExtKey 是可调用类型，返回的是给 hashtable 计算哈希值的部分
 * 对于 map 和 set，这个部分都是 _Value 的第一个值，即 std::get<0>(...)
 * 需要牢记的点：在 hashtable 中，_Key 是从 _Value 算出来的，不是外界传进来的
 * 
 * 对于完全自定义取键取值的hash_table，需要尽量避免operator[]的使用
 * 
 * 当哈希表处于 rehash 状态时，节点链表一定先遍历完 %_rehash_bucket 中的所有节点，再遍历 %_bucket 中的所有节点。
 * @implements
 * _bucket = [ 0 ,  1 ,  2 ,  3 ,  4 ]
 *             ↓              ↓
 *      [p] → [*]            [*] → [p]
 *             ↓
 *            [*]
 *             ↓
 *            {3}
 * 节点间使用双向指针链接
 * 
 *  - 当我们要在 _bucket[i] 下面查找/插入/删除节点时，是首先根据 key，确定元素在 _bucket 中的下标为 i，
 *    其次通过 _M_find_node 找到对应的节点 n。通过判空和比较，以确定查找结果/删除起点/插入位置。
 *    查找与删除操作较简单，基本不存在跨桶操作，仅需要注意节点间链接指针的问题。
 *    对于插入操作，需要分三种情况讨论：1. _bucket[i] == nul; 2. n == nullptr; 3. n != nullptr
 *    对于情况 1，我们以要在 _bucket[2] 下面插入节点为例进行讨论。首先找到前继节点，即 _bucket[0] 的尾节点，然后正常插入链表，并更新 _bucket[2]。
 *    对于情况 2，直接在 _bucket[i] 头部插入节点，并更新 _bucket[i]。
 *    对于情况 3，一般仅在 multi_table 中出现。正常插入链表即可。
 *  - 情况 1 中的前继节点如何查找？
 *    注意，节点间的链接，是仅为了方便进行节点 O(n) 遍历而使用的，我们没有必要让他们按 _bucket 数组下标的顺序进行链接。
 *    因此，我们可以直接令 _mark 节点作为前继节点。
 *  - 处于 rehash 状态时，节点链接如何实现？
 *    首先，为了方便 迭代器 遍历，我们不应该出现 _rehash_mark 一类的成员。
 *    节点链表与普通的节点插入一致。
 *    _rehash_process 会遍历完整个 _bucket 之后，回到 _mark 节点。
*/

// _Key = decltype(std::get<0>(_Value))
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, bool _Constant, typename _Hash, typename _Alloc> struct hash_node_iterator {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Constant, _Hash, _Alloc> self;

    typedef hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> _hash_table;

    typedef typename _hash_table::node_type node_type;
    typedef typename _hash_table::value_type value_type;
    typedef typename _hash_table::bucket_type bucket_type;
    typedef typename _hash_table::bucket_index bucket_index;

    typedef typename asp::conditional_t<_Constant, const node_type, node_type> _node_type;
    typedef typename asp::conditional_t<_Constant, const value_type, value_type> _value_type;

    _node_type* _cur = nullptr;
    // bucket_index _bi = _hash_table::_s_illegal_index;
    const _hash_table* _ht = nullptr;

    hash_node_iterator() = default;
    hash_node_iterator(const bucket_index& _i, const _hash_table* _h) : _cur(_h->_M_bucket(_i)), _ht(_h) {}
    hash_node_iterator(_node_type* _p, const _hash_table* _h) : _cur(_p), _ht(_h) {}
    // hash_node_iterator(_node_type* _p, const bucket_index& _i, const _hash_table* _h) : _cur(_p), _ht(_h), _bi(_i) {}
    hash_node_iterator(const self& _s) : _cur(_s._cur), _ht(_s._ht) {}
    hash_node_iterator(self&& _s) : _cur(std::move(_s._cur)), _ht(std::move(_s._ht)) {}
    virtual void _M_inc() {
        _cur = _cur->_next;
    }
    virtual void _M_dec() {
        _cur = _cur->_prev;
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
        this->_M_inc();
        return *this;
    }
    self operator++(int) {
        self _ret(*this);
        this->_M_inc();
        return _ret;
    }
    self& operator=(const self& _s) {
        _cur = _s._cur; _ht = _s._ht;
        return *this;
    }
    self& operator=(self&& _s) {
        _cur = std::move(_s._cur); _ht = std::move(_s._ht);
        return *this;
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
    bool _M_bucket_end() const {
        return _ht->_M_end_of_bucket(_cur);
    }
    template <typename _K, typename _V, typename _EK, bool _UK, typename _EV, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const hash_table<_K, _V, _EK, _UK, _EV, _H, _A>& _h);

    template <typename _K, typename _V, typename _EK, bool _UK, typename _EV, bool _C, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const hash_node_iterator<_K, _V, _EK, _UK, _EV, _C, _H, _A>& _h);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
 struct hash_iterator : public hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, false, _Hash, _Alloc> {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, false, _Hash, _Alloc> base;
    typedef hash_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> self;
    typedef hash_const_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> iterator;

    typedef typename base::_node_type node_type;
    typedef typename base::_hash_table _hash_table;
    typedef typename base::bucket_index bucket_index;

    hash_iterator() = default;
    hash_iterator(const bucket_index& _i, const _hash_table* _h) : base(_i, _h) {}
    hash_iterator(node_type* _p, const _hash_table* _h) : base(_p, _h) {}
    // hash_iterator(node_type* _p, const bucket_index& _i, const _hash_table* _h) : base(_p, _i, _h) {}
    hash_iterator(const self& _s) : base(_s) {}
    hash_iterator(self&& _s) : base(std::move(_s)) {}

    iterator _const_cast() const {
        return *this;
    }
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
 struct hash_const_iterator : public hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, true, _Hash, _Alloc> {
    typedef asp::forward_iterator_tag iterator_category;
    typedef hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, true, _Hash, _Alloc> base;
    typedef hash_const_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> self;
    typedef hash_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> iterator;

    typedef typename base::_node_type node_type;
    typedef typename base::_hash_table _hash_table;
    typedef typename base::bucket_index bucket_index;

    hash_const_iterator() = default;
    hash_const_iterator(const bucket_index& _i, const _hash_table* _h) : base(_i, _h) {}
    hash_const_iterator(const node_type* _p, const _hash_table* _h) : base(_p, _h) {}
    // hash_const_iterator(const node_type* _p, const bucket_index& _i, const _hash_table* _h) : base(_p, _i, _h) {}
    hash_const_iterator(const self& _s) : base(_s) {}
    hash_const_iterator(self&& _s) : base(std::move(_s)) {}
    hash_const_iterator(const iterator& _it) : base(_it._cur, _it._ht) {}
    hash_const_iterator(iterator&& _it) : base(std::move(_it._cur), std::move(_it._ht)) {}

    iterator _const_cast() const {
        return iterator(const_cast<node_type*>(this->_cur), this->_ht);
    }
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
 class hash_table : public hash_table_alloc<_Value, _Alloc> {
public:
    typedef hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> self;
    typedef hash_table_alloc<_Value, _Alloc> base;
    typedef hash_table_alloc<_Value, _Alloc> ht_alloc;
    typedef typename base::elt_allocator_type elt_allocator_type;
    typedef typename base::elt_alloc_traits elt_alloc_traits;
    typedef typename base::node_allocator_type node_allocator_type;
    typedef typename base::node_alloc_traits node_alloc_traits;
    typedef typename base::bucket_allocator_type bucket_allocator_type;
    typedef typename base::bucket_alloc_traits bucket_alloc_traits;
    typedef _ExtKey ext_key;
    typedef _ExtValue ext_value;

    typedef _Key key_type;
    typedef typename base::node_type node_type;
    typedef typename base::bucket_type bucket_type;
    typedef typename node_type::value_type value_type;
    typedef typename node_type::hash_code hash_code;
    typedef _Hash hasher;

    typedef hash_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> iterator;
    typedef hash_const_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc> const_iterator;

    typedef asp::conditional_t<_UniqueKey, std::pair<iterator, bool>, iterator> ireturn_type;

    typedef asso_container::type_traits<value_type, _UniqueKey> _ContainerTypeTraits;

    typedef typename _ContainerTypeTraits::insert_status insert_status;
    typedef typename _ContainerTypeTraits::ext_iterator ext_iterator;
    typedef typename _ContainerTypeTraits::mapped_type mapped_type;

    typedef rehash_policy::bucket_index bucket_index;
    typedef rehash_policy::bucket_id bucket_id;
    static const bucket_index _s_illegal_index;

    bucket_type* _buckets = nullptr;
    size_type _bucket_count = 0;
    bucket_type* _rehash_buckets = nullptr;  // only used in rehash
    size_type _rehash_bucket_count = 0;
    // node_type _before_begin;  // the node before @begin().
    size_type _element_count = 0;
    rehash_policy _rehash_policy;

    mutable node_type _mark; // _mark like in list

    _ExtKey _extract_key;
    _ExtValue _extract_value;

    template <typename _K, typename _V, typename _EK, bool _UK, typename _EV, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const hash_table<_K, _V, _EK, _UK, _EV, _H, _A>& _h);

    template <typename _K, typename _V, typename _EK, bool _UK, typename _EV, bool _C, typename _H, typename _A>
     friend struct hash_node_iterator;

public:
    hash_table();
    hash_table(bool _rehash_enabled);
    hash_table(const self& _ht);
    self& operator=(const self& _r);
    virtual ~hash_table();
    template <typename _NodeGen> void _M_assign(const self& _ht, const _NodeGen&);

    iterator begin() { return iterator(_M_begin(), this); }
    const_iterator cbegin() const { return const_iterator(_M_begin(), this); }
    iterator end() { return iterator(_M_end(), this); }
    const_iterator cend() const { return const_iterator(_M_end(), this); }
    size_type size() const { return _element_count; }
    bool empty() const { return _element_count == 0; }
    size_type bucket_count() const { return _M_in_rehash() ? _rehash_bucket_count : _bucket_count; }

    iterator find(const key_type& _k);
    const_iterator find(const key_type& _k) const;
    size_type count(const key_type& _k) const;
    void clear();
    ireturn_type insert(const value_type& _v);
    size_type erase(const key_type& _k);
    mapped_type& operator[](const key_type& _k);
    iterator update(const value_type& _v);
    hash_code _M_hash_code(const key_type& _k) const { return _Hash()(_k); }

    // used for test
    int check() const;

protected:
    void _M_init_mark() { _mark._prev = &_mark; _mark._next = &_mark; }
    void _M_deallocate_buckets() {
        base::_M_deallocate_buckets(_buckets, _bucket_count);
        base::_M_deallocate_buckets(_rehash_buckets, _rehash_bucket_count);
    }

    node_type* _M_begin() const { return _mark._next; }
    // const node_type* _M_end() const { return &_mark; }
    node_type* _M_end() const { return std::addressof(_mark); }
    bool _M_equals(const key_type& _k, hash_code _c, const node_type* _p) const { return _k == this->_extract_key(_p->val()); }
    /**
     * @param %_p must be a node in table.
     * @return whether %_p in %_M_bucket(_i), only check the key and index
    */
    bool _M_in_bucket(const node_type* const _p, const bucket_index& _i) const;
    /**
     * @param %_p must be in %_M_bucket(_i)
     * @return whether %_p is the last node of %_M_bucket(_i)
    */
    bool _M_end_of_bucket(const node_type* const _p, const bucket_index& _i) const;
    bool _M_end_of_bucket(const node_type* const _p) const;

    bool _M_valid_bucket_index(const bucket_index& _i) const;

    /**
     * @return index of %_k in %_bucket
    */
    bucket_index _M_index_in_bucket(const hash_code& _c) const { return _bucket_count ? bucket_index(0, _c % _bucket_count) : _s_illegal_index; }
    /**
     * @warning segment fault if _rehash_bucket == nullptr
     * @return index of %_k in %_rehash_bucket
    */
    bucket_index _M_index_in_rehash_bucket(const hash_code& _c) const { return _rehash_bucket_count ? bucket_index(1, _c % _rehash_bucket_count) : _s_illegal_index; }

    /**
     * @return bucket_index and pointer of potential node %{_k, _c}, return %end() if not existed.
    */
    std::pair<bucket_index, node_type*> _M_find_node(const key_type& _k, const hash_code& _c) const;
    std::pair<bucket_index, node_type*> _M_find_node_in_bucket(const key_type& _k, const hash_code& _c) const;
    /**
     * @return bucket_index and pointer of node %{_k, _c} insertion
    */
    std::pair<bucket_index, node_type*> _M_find_insertion_node(const key_type& _k, const hash_code& _c) const;

    // if %_M_bucket(_i) == %_p, return %_i
    bucket_index _M_find_head_node(const key_type& _k, const node_type* const _p) const;
    // if %_M_bucket(_i) == %_p, return %_i
    bucket_index _M_find_head_node(const hash_code& _c, const node_type* const _p) const;

    // void _M_next_bucket_index(bucket_index& _i) const;
    bucket_type _M_bucket(const bucket_index& _i) const;
    bucket_type& _M_bucket_ref(const bucket_index& _i) const;
    // find node {_k, _c} in _bucket[_i]
    node_type* _M_find_node_in_given_bucket(const bucket_index& _i, const key_type& _k, hash_code _c) const;
    bool _M_given_node_in_given_bucket(const bucket_index& _i, const node_type* const _x) const;

    bool _M_valid_bucket_index_unguard(const bucket_index& _i) const;
    bucket_type _M_bucket_unguard(const bucket_index& _i) const;
    // find node {_k, _c} in _bucket[_i], recommended to use only in rehash
    node_type* _M_find_node_in_given_bucket_unguard(const bucket_index& _i, const key_type& _k, hash_code _c) const;
    bool _M_given_node_in_given_bucket_unguard(const bucket_index& _i, const node_type* const _x) const;

    // hook %_n after %_p
    void _M_hook_node(node_type* const _p, node_type* const _n) const;
    void _M_unhook_node(node_type* const _n) const;

    void _M_insert_null_bucket(const bucket_index& _i, node_type* _n);
    void _M_insert_bucket_begin(const bucket_index& _i, node_type* _n);

    // insert allocated and constructed _n into _bucket[_i]
    iterator _M_insert_unique_node(const bucket_index& _i, node_type* _p, hash_code _c, node_type* _n);
    // insert allocated and constructed _n into _bucket[_i]
    iterator _M_insert_multi_node(const bucket_index& _i, node_type* _p, hash_code _c, node_type* _n);
    /// implement
    iterator _M_insert_unique(const bucket_index& _i, node_type* _p, hash_code _c, const value_type& _v);
    iterator _M_insert_multi(const bucket_index& _i, node_type* _p, hash_code _c, const value_type& _v);
    std::pair<iterator, bool> _M_insert(const value_type& _v, asp::true_type);
    iterator _M_insert(const value_type& _v, asp::false_type);
    size_type _M_erase(const key_type& _k, asp::true_type);
    size_type _M_erase(const key_type& _k, asp::false_type);
    iterator _M_update(const value_type& _v, asp::true_type);
    iterator _M_update(const value_type& _v, asp::false_type);

    /// rehash policy
    std::pair<bool, size_type> _M_need_rehash(size_type _ins = 1) const { return this->_rehash_policy.need_rehash(_bucket_count, _element_count, _ins); }
    bool _M_in_rehash() const { return this->_rehash_policy._in_rehash; }
    virtual void _M_start_rehash(size_type _next_bkt);
    virtual void _M_finish_rehash();
    virtual task_status _M_step_rehash(size_type _step = 1);
    /**
     * @brief execute a rehash if necessary.
     * @details function would invalidate iterator, bucket_indx.
     * */
    virtual void _M_rehash_if_required();
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::hash_table() {
    size_type _s = _prime_list[0];
    this->_buckets = this->_M_allocate_buckets(_s);
    this->_bucket_count = _s;
    this->_M_init_mark();
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::hash_table(bool _rehash_enabled) : _rehash_policy(_rehash_enabled) {
    size_type _s = _prime_list[0];
    this->_buckets = this->_M_allocate_buckets(_s);
    this->_bucket_count = _s;
    this->_M_init_mark();
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::hash_table(const self& _ht)
: base(_ht), _buckets(nullptr), _bucket_count(_ht._bucket_count)
, _rehash_buckets(nullptr), _rehash_bucket_count(_ht._rehash_bucket_count)
, _element_count(_ht._element_count), _rehash_policy(_ht._rehash_policy), _extract_key(_ht._extract_key) {
    this->_M_init_mark();
    _M_assign(_ht, [this](const node_type* _n) {
        return this->_M_allocate_node(*_n);
    });
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::operator=(const self& _r)
-> self& {
    if (&_r == this) return *this;
    clear();
    _M_assign(_r, [this](const node_type* _n) {
        return this->_M_allocate_node(*_n);
    });
    _element_count = _r.size();
    _rehash_policy = _r._rehash_policy;
    _bucket_count = _r._bucket_count;
    _rehash_bucket_count = _r._rehash_bucket_count;
    return *this;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::~hash_table() {
    clear();
    this->_M_deallocate_buckets();
    _buckets = nullptr; _bucket_count = 0;
    _rehash_buckets = nullptr; _rehash_bucket_count = 0;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
template <typename _NodeGen> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_assign(const self& _ht, const _NodeGen& _gen) -> void {
    bucket_type* _t_buckets = nullptr;
    bucket_type* _t_rehash_buckets = nullptr;
    if (_buckets == nullptr) {
        _buckets = _t_buckets = this->_M_allocate_buckets(_ht._bucket_count);
        _rehash_buckets = _t_rehash_buckets = this->_M_allocate_buckets(_ht._rehash_bucket_count);
    }
    node_type* _prev = _M_end();
    for (node_type* _head = _ht._M_begin(); _head != _ht._M_end();) {
        const key_type& _hk = _ht._extract_key(_head->val());
        const bucket_index _i = _ht._M_find_head_node(_hk, _head);
        for (node_type* _cur = _head;; _cur = _cur->_next) {
            node_type* _p = _gen(_cur);
            this->_M_hook_node(_prev, _p);
            _prev = _p;
            if (_M_bucket(_i) == nullptr) {
                _M_bucket_ref(_i) == _p;
            }
            if (_ht._M_end_of_bucket(_cur)) {
                _head = _cur->_next; break;
            }
        }
    }
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_in_bucket(const node_type* const _p, const bucket_index& _i) const
-> bool {
    if (_p == _M_end() || _p == nullptr) return false;
    const key_type& _k = this->_extract_key(_p->val());
    const hash_code& _c = this->_M_hash_code(_k);
    if (_i.first == 0) {
        return _M_index_in_bucket(_c) == _i;
    }
    else {
        if (!_M_in_rehash()) return false;
        return _M_index_in_rehash_bucket(_c) == _i;
    }
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_end_of_bucket(const node_type* const _p, const bucket_index& _i) const
-> bool {
    if (_p == _M_end() || _p == nullptr || _i.first == -1) return true;
    if (_p->_next == _M_end()) return true;
    const key_type& _nk = this->_extract_key(_p->_next->val());
    const hash_code& _nc = this->_M_hash_code(_nk);
    if (_i.first == 0) {
        return _M_index_in_bucket(_nc) != _i;
    }
    else { // _i.first == 1
        // if (!_M_in_rehash()) return false;
        assert(_M_in_rehash());
        if (_M_index_in_rehash_bucket(_nc) != _i) return true;
        const auto _i2 = _M_index_in_bucket(_nc);
        if (_M_bucket(_i2) == _p->_next) {
            return true;
        }
        return false;
    }
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_end_of_bucket(const node_type* const _p) const
-> bool {
    if (_p == _M_end() || _p == nullptr) return true;
    if (_p->_next == _M_end()) return true;
    const key_type& _nk = this->_extract_key(_p->_next->val());
    const hash_code& _nc = this->_M_hash_code(_nk);
    const bucket_index& _i = _M_index_in_bucket(_nc);
    if (_M_bucket(_i) == _p->_next) return true;
    const bucket_index& _ri = _M_index_in_rehash_bucket(_nc);
    if (_M_bucket(_ri) == _p->_next) return true;
    return false;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_valid_bucket_index(const bucket_index& _i) const
-> bool {
    if (_i.first == 0) {
        return _i.second >= 0 && _i.second < this->_bucket_count;
    }
    if (_i.first == 1 && this->_M_in_rehash()) {
        return _i.second >= 0 && _i.second < this->_rehash_bucket_count;
    }
    return false;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_find_node(const key_type& _k, const hash_code& _c) const
-> std::pair<bucket_index, node_type*> {
    // search in %_bucket first, and %_rehash_bucket if in rehash
    const bucket_index _i = _M_index_in_bucket(_c);
    node_type* const _n = _M_find_node_in_given_bucket(_i, _k, _c);
    if (_n != nullptr) {
        return std::make_pair(_i, _n);
    }
    if (_M_in_rehash()) {
        const bucket_index _i = _M_index_in_rehash_bucket(_c);
        node_type* const _n = _M_find_node_in_given_bucket(_i, _k, _c);
        if (_n != nullptr) {
            return std::make_pair(_i, _n);
        }
    }
    return std::make_pair(_s_illegal_index, nullptr);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_find_node_in_bucket(const key_type& _k, const hash_code& _c) const
-> std::pair<bucket_index, node_type*> {
    // search in %_bucket first, and %_rehash_bucket if in rehash
    const bucket_index _i = _M_index_in_bucket(_c);
    node_type* const _n = _M_find_node_in_given_bucket(_i, _k, _c);
    if (_n != nullptr) {
        return std::make_pair(_i, _n);
    }
    return std::make_pair(_s_illegal_index, nullptr);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_find_insertion_node(const key_type& _k, const hash_code& _c) const
-> std::pair<bucket_index, node_type*> {
    const bucket_index _i = _M_in_rehash() ?
        _M_index_in_rehash_bucket(_c) :
        _M_index_in_bucket(_c);
    node_type* const _n = _M_find_node_in_given_bucket(_i, _k, _c);
    // if (_n == nullptr) _n = _M_end();
    return std::make_pair(_i, _n);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_find_head_node(const key_type& _k, const node_type* const _p) const
-> bucket_index {
    return _M_find_head_node(this->_M_hash_code(_k), _p);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_find_head_node(const hash_code& _c, const node_type* const _p) const
-> bucket_index {
    if (_M_in_rehash()) {
        const bucket_index _i = _M_index_in_rehash_bucket(_c);
        if (_M_bucket(_i) == _p) return _i;
    }
    const bucket_index _i = _M_index_in_bucket(_c);
    if (_M_bucket(_i) == _p) return _i;
    return _s_illegal_index;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_bucket(const bucket_index& _i) const
-> bucket_type {
    if (!_M_valid_bucket_index(_i)) {
        return nullptr;
    }
    if (_i.first == 1 && _M_in_rehash()) {
        return this->_rehash_buckets[_i.second];
    }
    else if (_i.first == 0) {
        return this->_buckets[_i.second];
    }
    return nullptr;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_bucket_ref(const bucket_index& _i) const
-> bucket_type& {
    if (_i.first == 1) {
        return this->_rehash_buckets[_i.second];
    }
    return this->_buckets[_i.second];
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_find_node_in_given_bucket(const bucket_index& _i, const key_type& _k, hash_code _c) const
-> node_type* {
    node_type* _p = this->_M_bucket(_i);
    if (_p == nullptr) return nullptr;
    for (; _p != _M_end(); _p = _p->_next) {
        if (this->_M_equals(_k, _c, _p)) {
            return _p;
        }
        if (_M_end_of_bucket(_p)) break;
        // if (_M_end_of_bucket(_p, _i)) break;
    }
    return nullptr;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_given_node_in_given_bucket(const bucket_index& _i, const node_type* const _x) const
-> bool {
    node_type* _p = this->_M_bucket(_i);
    if (_p == nullptr) return false;
    for (; _p != _M_end(); _p = _p->_next) {
        if (_p == _x) {
            return true;
        }
        if (_M_end_of_bucket(_p)) break;
        // if (_M_end_of_bucket(_p, _i)) break;
    }
    return false;
};

/// unguard function, recommend to use only in rehash
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_valid_bucket_index_unguard(const bucket_index& _i) const
-> bool {
    if (_i.first == 0) {
        return _i.second >= 0 && _i.second < this->_bucket_count;
    }
    if (_i.first == 1) {
        return _i.second >= 0 && _i.second < this->_rehash_bucket_count;
    }
    return false;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_bucket_unguard(const bucket_index& _i) const
-> bucket_type {
    if (!_M_valid_bucket_index_unguard(_i)) {
        return nullptr;
    }
    if (_i.first == 1) {
        return this->_rehash_buckets[_i.second];
    }
    else if (_i.first == 0) {
        return this->_buckets[_i.second];
    }
    return nullptr;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_find_node_in_given_bucket_unguard(const bucket_index& _i, const key_type& _k, hash_code _c) const
-> node_type* {
    node_type* _p = this->_M_bucket_unguard(_i);
    if (_p == nullptr) return nullptr;
    for (; _p != _M_end(); _p = _p->_next) {
        if (this->_M_equals(_k, _c, _p)) {
            return _p;
        }
        if (_M_end_of_bucket(_p)) break;
        // if (_M_end_of_bucket(_p, _i)) break;
    }
    return nullptr;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_given_node_in_given_bucket_unguard(const bucket_index& _i, const node_type* const _x) const
-> bool {
    node_type* _p = this->_M_bucket_unguard(_i);
    if (_p == nullptr) return false;
    for (; _p != _M_end(); _p = _p->_next) {
        if (_p == _x) {
            return true;
        }
        if (_M_end_of_bucket(_p)) break;
        // if (_M_end_of_bucket(_p, _i)) break;
    }
    return false;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_hook_node(node_type* const _p, node_type* const _n) const
-> void {
    _n->_next = _p->_next;
    _n->_prev = _p;
    _p->_next->_prev = _n;
    _p->_next = _n;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_unhook_node(node_type* const _n) const
-> void {
    assert(_n != _M_end());
    node_type* const _p = _n->_prev; assert(_p != _n);
    _p->_next = _n->_next;
    _p->_next->_prev = _p;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_insert_null_bucket(const bucket_index& _i, node_type* _n)
-> void {
    _M_hook_node(&_mark, _n);
    this->_M_bucket_ref(_i) = _n;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_insert_bucket_begin(const bucket_index& _i, node_type* _n)
-> void {
    node_type* _hint = this->_M_bucket(_i);
    _hint = _hint->_prev;
    _M_hook_node(_hint, _n);
    this->_M_bucket_ref(_i) = _n;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_insert_unique_node(const bucket_index& _i, node_type* _p, hash_code _c, node_type* _n)
-> iterator {
    // _p == nullptr
    _n->_hash_code = _c;
    if (this->_M_bucket(_i) == nullptr) {
        this->_M_insert_null_bucket(_i, _n);
    }
    else {
        this->_M_insert_bucket_begin(_i, _n);
    }
    return iterator(_n, this);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::
_M_insert_multi_node(const bucket_index& _i, node_type* _p, hash_code _c, node_type* _n)
-> iterator {
    _n->_hash_code = _c;
    const node_type* _hint = this->_M_bucket(_i);
    if (_hint == nullptr) {
        this->_M_insert_null_bucket(_i, _n);
    }
    else if (_p == nullptr) {
        this->_M_insert_bucket_begin(_i, _n);
    }
    else {
        this->_M_hook_node(_p, _n);
    }
    return iterator(_n, this);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_insert_unique(const bucket_index& _i, node_type* _p, hash_code _c, const value_type& _v)
-> iterator {
    // _p == nullptr
    node_type* _n = this->_M_allocate_node(_v);
    ++_element_count;
    return _M_insert_unique_node(_i, _p, _c, _n);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_insert_multi(const bucket_index& _i, node_type* _p, hash_code _c, const value_type& _v)
-> iterator {
    node_type* _n = this->_M_allocate_node(_v);
    ++_element_count;
    return _M_insert_multi_node(_i, _p, _c, _n);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_insert(const value_type& _v, asp::true_type)
-> std::pair<iterator, bool> {
    const key_type _k = this->_extract_key(_v);
    const hash_code _c = this->_M_hash_code(_k);
    const auto _pr = this->_M_find_node(_k, _c);
    if (_pr.second != nullptr) {
        return {iterator(_pr.first, this), false};
    }
    const auto _ipr = this->_M_find_insertion_node(_k, _c);
    if (_ipr.second != nullptr) {
        return {iterator(_ipr.first, this), false};
    }
    return {this->_M_insert_unique(_ipr.first, _ipr.second, _c, _v), true};
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_insert(const value_type& _v, asp::false_type)
-> iterator {
    const key_type _k = this->_extract_key(_v);
    const hash_code _c = this->_M_hash_code(_k);
    const auto _p = this->_M_find_insertion_node(_k, _c);
    return this->_M_insert_multi(_p.first, _p.second, _c, _v);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_erase(const key_type& _k, asp::true_type)
-> size_type {
    const hash_code _c = this->_M_hash_code(_k);
    const auto _pr = this->_M_find_node(_k, _c);
    const bucket_index _i = _pr.first;
    if (!this->_M_valid_bucket_index(_i)) { return 0; }
    node_type* const _n = _pr.second;
    assert(_n != nullptr && _n != _M_end());
    node_type* _hint = this->_M_bucket(_i);
    if (_hint == nullptr) { return 0; }
    if (_hint == _n) { // %_n is the head node
        if (_M_end_of_bucket(_hint)) {
        // if (_M_end_of_bucket(_hint, _i)) {
            this->_M_bucket_ref(_i) = nullptr;
        }
        else {
            _hint = _n->_next;
            this->_M_bucket_ref(_i) = _hint;
        }
    }
    _M_unhook_node(_n);
    this->_M_deallocate_node(_n);
    --_element_count;
    return 1;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_erase(const key_type& _k, asp::false_type)
-> size_type {
    // node to be erased may exist in both bucket
    const hash_code _c = this->_M_hash_code(_k);
    auto erase_in_given_bucket = [&](const bucket_index& _i) -> size_type {
        if (!this->_M_valid_bucket_index(_i)) return 0;
        node_type* _p = this->_M_find_node_in_given_bucket(_i, _k, _c);
        if (_p == nullptr) return 0;
        const bool _p_head = this->_M_bucket(_i) == _p; // only if %_p_head, we need to update %_M_bucket_ref(_i)
        bool _p_tail = false;
        size_type _cnt = 0;
        while (_p != this->_M_end() && this->_M_equals(_k, _c, _p) && !_p_tail) {
            this->_M_unhook_node(_p);
            node_type* const _s = _p;
            if (_M_end_of_bucket(_p)) _p_tail = true;
            // if (_M_end_of_bucket(_p, _i)) _p_tail = true;
            _p = _p->_next;
            this->_M_deallocate_node(_s);
            ++_cnt;
            --this->_element_count;
        }
        if (_p_head) {
            if (_p != this->_M_end() && !_p_tail) {
                this->_M_bucket_ref(_i) = _p;
            }
            else {
                this->_M_bucket_ref(_i) = nullptr;
            }
        }
        return _cnt;
    };
    size_type _remove_cnt = 0;
    const bucket_index _bi = this->_M_index_in_bucket(_c);
    const bucket_index _rbi = this->_M_index_in_rehash_bucket(_c);
    _remove_cnt += erase_in_given_bucket(_bi);
    _remove_cnt += erase_in_given_bucket(_rbi);
    return _remove_cnt;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_update(const value_type& _v, asp::true_type)
-> iterator {
    _M_erase(_extract_key(_v), asp::true_type());
    return this->_M_insert(_v, asp::true_type());
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_update(const value_type& _v, asp::false_type)
-> iterator {
    return this->_M_insert(_v, asp::false_type());
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::find(const key_type& _k)
-> iterator {
    this->_M_rehash_if_required();

    hash_code _c = this->_M_hash_code(_k);
    node_type* _p = this->_M_find_node(_k, _c).second;
    if (_p == nullptr) _p = _M_end();
    return iterator(_p, this);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::find(const key_type& _k) const
-> const_iterator {
    hash_code _c = this->_M_hash_code(_k);
    node_type* _p = this->_M_find_node(_k, _c).second;
    if (_p == nullptr) _p = _M_end();
    return const_iterator(_p, this);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::count(const key_type& _k) const
-> size_type {
    hash_code _c = this->_M_hash_code(_k);
    auto count_in_given_bucket = [&](const bucket_index& _i) -> size_type {
        if (!this->_M_valid_bucket_index(_i)) return 0;
        node_type* _p = this->_M_find_node_in_given_bucket(_i, _k, _c);
        if (_p == nullptr) return 0;
        size_type _cnt = 0;
        while (_p != this->_M_end() && this->_M_equals(_k, _c, _p)) {
            ++_cnt;
            if (_M_end_of_bucket(_p)) break;
            // if (_M_end_of_bucket(_p, _i)) break;
            _p = _p->_next;
        }
        return _cnt;
    };
    size_type _cnt = 0;
    const bucket_index _bi = this->_M_index_in_bucket(_c);
    const bucket_index _rbi = this->_M_index_in_rehash_bucket(_c);
    _cnt += count_in_given_bucket(_bi);
    _cnt += count_in_given_bucket(_rbi);
    return _cnt;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::clear()
-> void {
    // if in rehash, stop rehash force, which would destroy the data.
    if (_M_in_rehash()) { this->_M_finish_rehash(); }

    for (node_type* _p = _M_begin(); _p != _M_end();) {
        node_type* const _s = _p;
        _p = _p->_next;
        this->_M_deallocate_node(_s);
    }
    memset(_buckets, 0, _bucket_count * sizeof(bucket_type));
    memset(_rehash_buckets, 0, _rehash_bucket_count * sizeof(bucket_type));

    this->_M_init_mark();
    this->_element_count = 0;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::insert(const value_type& _v)
-> ireturn_type {
    this->_M_rehash_if_required();

    return this->_M_insert(_v, asp::bool_t<_UniqueKey>());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::erase(const key_type& _k)
-> size_type {
    this->_M_rehash_if_required();

    return this->_M_erase(_k, asp::bool_t<_UniqueKey>());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::operator[](const key_type& _k)
-> mapped_type& {
    hash_code _c = this->_M_hash_code(_k);
    const auto _pr = this->_M_find_node(_k, _c);
    node_type* _p = _pr.second;
    if (_p == nullptr) {
        const auto _ipr = this->_M_find_insertion_node(_c);
        _p = this->_M_allocate_node(std::piecewise_construct, std::tuple<const key_type&>(_k), std::tuple<>());
        return _extract_value(*(this->_M_insert_unique_node(_ipr.first, _ipr.second, _c, _p)));
    }
    return _extract_value(_p->val());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::update(const value_type& _v)
-> iterator {
    this->_M_rehash_if_required();

    return this->_M_update(_v, asp::bool_t<_UniqueKey>());
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::check() const
-> int {
    /**
     * @return 0 = normal
     * 1 = duplicate value in unique container;
     * 2 = the same value(s) are stored not adjacent;
     * 3 = not in rehash, but %_rehash_buckets etc haven't been reset yet;
     * 4 = the number of traversed nodes is not equal to %_element_count;
     * 5 = error in node link
     * 6 = node in %_rehash_bucket when not in rehash
     * 7 = node in wrong bucket
     * 8 = non-head node was traversed firstly
    */
#ifdef _HASH_TABLE_CHECK_
    size_type _counter = 0;
    std::unordered_set<key_type> _uset;
    const bool _unique = _UniqueKey;
    asp::decay_t<key_type> _last_value;
    if (!_M_in_rehash()) {
        if (_rehash_buckets != nullptr || _rehash_bucket_count != 0) {
            return 3;
        }
    }
    for (node_type* _p = _M_begin(); _p != _M_end();) {
        if (_counter > _element_count) return 4;
        node_type* const _head = _p;
        const key_type& _hk = this->_extract_key(_head->val());
        const bucket_index _i = _M_find_head_node(_hk, _head);

        if (_i.first == -1) {
            return 8;
        }

        if (_i.first == 1 && !_M_in_rehash()) return 6;
        while (1) {
            if (_counter > _element_count) return 4;
            const key_type& _k = this->_extract_key(_p->val());
            const hash_code& _c = this->_M_hash_code(_k);
            if (_unique) {
                node_type* const _fn = _M_find_node_in_given_bucket_unguard(_i, _k, _c);
                if (_fn != _p) {
                    std::cout << "_i = " << _i << ", _k = " << _k << std::endl;
                    std::cout << "&_fn = " << _fn << ", &_p = " << _p << std::endl;
                    if (_fn != nullptr)
                        std::cout << "_fn = " << _fn->val() << ", _p = " << _p->val() << std::endl;
                    else
                        std::cout << "_fn = null, _p = " << _p->val() << std::endl;
                    return 7;
                }
            }
            else {
                const bool _fn = _M_given_node_in_given_bucket_unguard(_i, _p);
                if (!_fn)
                    return 7;
            }
            if (_p->_next->_prev != _p) return 5;
            ++_counter;
            if (_uset.count(_k)) {
                if (_unique) return 1;
                // if (_last_value != _k) return 2;
            }
            _uset.insert(_k);
            _last_value = _k;
            if (_M_end_of_bucket(_p)) {
            // if (_M_end_of_bucket(_p, _i)) {
                break;
            }
            _p = _p->_next;
        }
        _p = _p->_next;
    }
    if (_counter != _element_count) {
        return 4;
    }
#endif // _HASH_TABLE_CHECK_
    return 0;
};

/// rehash_policy
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_start_rehash(size_type _next_bkt)
-> void {
    if (this->_M_in_rehash()) { return; }
    this->_rehash_policy._in_rehash = true;
    _rehash_buckets = this->_M_allocate_buckets(_next_bkt);
    _rehash_bucket_count = _next_bkt;
    _rehash_policy._cur_process = _M_index_in_bucket(_M_begin()->_hash_code);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_finish_rehash()
-> void {
    _rehash_policy._in_rehash = false;
    _rehash_policy._cur_process = _s_illegal_index;
    if (_rehash_buckets == nullptr) {
        _rehash_bucket_count = 0;
        return;
    }
    std::swap(_buckets, _rehash_buckets);
    std::swap(_bucket_count, _rehash_bucket_count);

    base::_M_deallocate_buckets(_rehash_buckets, _rehash_bucket_count);
    _rehash_buckets = nullptr;
    _rehash_bucket_count = 0;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_step_rehash(size_type _step)
-> task_status {
    if (!this->_M_in_rehash()) { return task_status::__FAILED__; }
    if (!this->_M_valid_bucket_index(_rehash_policy._cur_process)) { return task_status::__FAILED__; }
    if (_rehash_policy._cur_process.first != 0) { return task_status::__FAILED__; }
    while (_step--) {
        const bucket_index  _i = _rehash_policy._cur_process;
        node_type* _hint = nullptr;
        for (_hint = this->_M_bucket(_i); _hint != nullptr && _hint != _M_end();) {
            node_type* _next_hint = _hint->_next;
            // move %_hint to %_rehash_buckets
            const key_type& _hk = this->_extract_key(_hint->val());
            const hash_code& _hc = this->_M_hash_code(_hk);
            const auto _ipr = this->_M_find_insertion_node(_hk, _hc);
            const bool _hint_end_of_bucket = _M_end_of_bucket(_hint);
            // const bool _hint_end_of_bucket = _M_end_of_bucket(_hint, _i);
            if (_ipr.second != _hint) {
                this->_M_unhook_node(_hint);
                // can't append directly
                if (_UniqueKey) {
                    _M_insert_unique_node(_ipr.first, _ipr.second, _hint->_hash_code, _hint);
                }
                else {
                    _M_insert_multi_node(_ipr.first, _ipr.second, _hint->_hash_code, _hint);
                }
            }
            // this->_M_deallocate_node(_hint);  // we can't deallocate node here !!!
            if (_hint_end_of_bucket) {
                // _M_bucket_ref(_i) = nullptr;
                _hint = _next_hint;
                break;
            }
            _M_bucket_ref(_i) = _next_hint;
            _hint = _next_hint;
        }
        // we've **moved** all nodes in _M_bucket[_i] to _rehash_bucket
        this->_M_bucket_ref(_i) = nullptr;
        if (_hint == nullptr || _hint == _M_end()) {
            return task_status::__COMPLETED__;
        }
        const key_type& _k = this->_extract_key(_hint->val());
        const hash_code& _c = this->_M_hash_code(_k);
        // _rehash_policy._cur_process = _M_find_node_in_bucket(_k, _c).first;
        _rehash_policy._cur_process = _M_find_head_node(_c, _hint);
        if (this->_M_valid_bucket_index(_rehash_policy._cur_process) && _rehash_policy._cur_process.first == 1) {
            assert(false);
            return task_status::__COMPLETED__;
        }
    };
    return task_status::__NORMAL__;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_M_rehash_if_required()
-> void {
    if (!this->_M_in_rehash()) {    
        auto _rehash_info = this->_M_need_rehash();
        if (_rehash_info.first) {
            this->_M_start_rehash(_rehash_info.second);
        }
        else if (_rehash_info.second > 0) { // need to resize not rehash
            this->_M_start_rehash(_rehash_info.second);
            this->_M_step_rehash(_bucket_count);
            this->_M_finish_rehash();
            return;
        }
    }
    if (this->_M_in_rehash()) {
        auto _ret = this->_M_step_rehash();
        if (_ret == task_status::__COMPLETED__) {
            this->_M_finish_rehash();
        }
    }
};


/// constexpr static const member
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc>
constexpr const rehash_policy::bucket_index
hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>::_s_illegal_index
 = bucket_index(-1, 0);


/// output stream
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
operator<<(std::ostream& os, const hash_table<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Hash, _Alloc>& _h)
-> std::ostream& {
    os << '[';
    for (auto p = _h.cbegin(); p != _h.cend();) {
        os << p;
        bool _be = static_cast<hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, true, _Hash, _Alloc>>(p)._M_bucket_end();
        if (++p != _h.cend()) {
            os << (_be ? "; " : ", ");
        }
    }
    os << ']';
    return os;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _ExtValue, bool _Constant, typename _Hash, typename _Alloc> auto
operator<<(std::ostream& os, const hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Constant, _Hash, _Alloc>& _h)
-> std::ostream& {
    using ht = typename hash_node_iterator<_Key, _Value, _ExtKey, _UniqueKey, _ExtValue, _Constant, _Hash, _Alloc>::_hash_table;
    if (_h) {
        os << obj_string::_M_obj_2_string(*_h);
        // if (ht::kv_self::value) {
        //     os << typename ht::ext_value()(*_h);
        // }
        // else {
        //     // os << "{" << _h._ht->_extract_key(*_h) << ", " << typename ht::ext_value()(*_h) << "}";
        //     os << "{" << typename ht::_ExtKey()(*_h) << ", " << typename ht::ext_value()(*_h) << "}";
        // }
    }
    else {
        os << "null";
    }
    return os;
};

};

#endif  // _ASP_HASH_TABLE_HPP_