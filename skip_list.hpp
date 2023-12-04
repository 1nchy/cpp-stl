#ifndef _ASP_SKIP_LIST_HPP_
#define _ASP_SKIP_LIST_HPP_

#include "basic_param.hpp"
#include "skip_list_node.hpp"
#include "associative_container_aux.hpp"
#include "random.hpp"

#include <cstring>
#include <unordered_map>

namespace asp {

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey,
 typename _Comp = std::less<_Key>, typename _Alloc = std::allocator<_Value>> class skip_list;
template <typename _Value, typename _Alloc = std::allocator<_Value>> struct skip_list_alloc;

template <typename _Value, typename _Alloc> struct skip_list_alloc : public _Alloc {
    typedef skip_list_node<_Value> node_type;
    typedef node_type* map_type;
    typedef _Alloc elt_allocator_type;
    typedef std::allocator_traits<elt_allocator_type> elt_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<node_type> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<node_type*> map_allocator_type;
    typedef std::allocator_traits<map_allocator_type> map_alloc_traits;

    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type*>(this); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<const elt_allocator_type*>(this); }
    node_allocator_type _M_get_node_allocator() const { return node_allocator_type(_M_get_elt_allocator()); }
    map_allocator_type _M_get_map_allocator() const { return map_allocator_type(_M_get_elt_allocator()); }

    node_type* _M_allocate_node(const node_type& _x) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, _x.val());
        return _p;
    }
    template <typename... _Args> node_type* _M_allocate_node(_Args&&... _args) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_node(node_type* _p) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        if (_p->_next != nullptr)
            _M_deallocate_map(_p->_next, _p->_height);
        node_alloc_traits::destroy(_node_alloc, _p);
        node_alloc_traits::deallocate(_node_alloc, _p, 1);
    }

    map_type* _M_allocate_map(size_type _n) {
        map_allocator_type _map_alloc = _M_get_map_allocator();
        map_type* _p = map_alloc_traits::allocate(_map_alloc, _n);
        bzero(_p, sizeof(map_type) * _n);
        return _p;
    }
    void _M_deallocate_map(map_type* _p, size_type _n) {
        map_allocator_type _map_alloc = _M_get_map_allocator();
        map_alloc_traits::deallocate(_map_alloc, _p, _n);
    }
};

/**
 * @brief 
 * @details
 * // skip list :
 *        begin                                      end
 *   ┌─┐                                             ┌─┐
 *    3          ┌→┐($)                     
 *    2           → (8)                  ┌→┐($)           (sub lists)
 *    1           → (5)      ┌→┐(8)       → ($)        
 *    0    ┌→┐    →    ┌→┐    →    ┌→┐    →    ┌→┐        (main list)
 *   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘
 *  _mark   1     3     5     5     7     8     9   _mark
 * // skip list
 * 
 * 1. Node height monotonically decreases ↓ when looking up.
 * 2. The height of node is fixed when it's inserted, and wouldn't change during insertion or deletion of other nodes.
*/
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
class skip_list : public skip_list_alloc<_Value, _Alloc> {
public:
    typedef skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc> self;
    typedef skip_list_alloc<_Value, _Alloc> base;
    typedef skip_list_alloc<_Value, _Alloc> skl_alloc;
    typedef typename skl_alloc::elt_allocator_type elt_allocator_type;
    typedef typename skl_alloc::elt_alloc_traits elt_alloc_traits;
    typedef typename skl_alloc::node_allocator_type node_allocator_type;
    typedef typename skl_alloc::node_alloc_traits node_alloc_traits;

    typedef _Key key_type;
    typedef _Comp key_compare;
    typedef typename base::node_type node_type;
    typedef typename node_type::value_type value_type;
    typedef typename skl_alloc::map_type map_type;

    typedef skip_list_iterator<value_type> iterator;
    typedef skip_list_const_iterator<value_type> const_iterator;

    typedef asp::conditional_t<_UniqueKey, std::pair<iterator, bool>, iterator> ireturn_type;
    typedef asso_container::type_traits<value_type, _UniqueKey> _ContainerTypeTraits;

    typedef typename _ContainerTypeTraits::insert_status insert_status;
    typedef typename _ContainerTypeTraits::ext_iterator ext_iterator;
    typedef typename _ContainerTypeTraits::ext_value ext_value;
    typedef typename _ContainerTypeTraits::mapped_type mapped_type;
    typedef _ExtKey ext_key;

    node_type _mark;
    size_type _m_element_count = 0;
    _Comp _m_key_compare;

    static const value_type& _S_value(const node_type* _x) { return _x->val(); }
    static key_type _S_key(const node_type* _x) { return _ExtKey()(_x->val()); }
    static key_type _S_key(const value_type& _v) { return _ExtKey()(_v); }

    template <typename _K, typename _V, typename _EK, bool _UK, typename _C, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const skip_list<_K, _V, _EK, _UK, _C, _A>& _sl);
public:
    skip_list() { _M_set_node_height(&_mark, _S_max_height); _M_init_mark(); }
    virtual ~skip_list();

    iterator begin() { return iterator(_M_begin()); }
    const_iterator cbegin() const { return const_iterator(_M_begin()); }
    iterator end() { return iterator(_M_end()); }
    const_iterator cend() const { return const_iterator(_M_end()); }
    size_type size() const { return _m_element_count; }
    bool empty() const { return _m_element_count == 0; }

    const_iterator find(const key_type& _k) const;
    size_type count(const key_type& _k) const;
    void clear();
    ireturn_type insert(const value_type& _v);
    size_type erase(const key_type& _k);

    iterator lower_bound(const key_type& _k) { return _M_lower_bound(_M_begin(), _M_end(), _k); }
    const_iterator lower_bound(const key_type& _k) const { return _M_lower_bound(_M_begin(), _M_end(), _k); }
    iterator upper_bound(const key_type& _k) { return _M_upper_bound(_M_begin(), _M_end(), _k); }
    const_iterator upper_bound(const key_type& _k) const { return _M_upper_bound(_M_begin(), _M_end(), _k); }
    // std::pair<iterator, iterator> equal_range(const key_type& _k);
    // std::pair<const_iterator, const_iterator> equal_range(const key_type& _k) const;

    //used for test
    int check() const;

protected:
    static constexpr const size_type _S_max_height = 8;
    static constexpr const double _S_height_prob = 0.5;

    bool _log_height = true;

    void _M_init_mark() { _mark._prev = &_mark; _mark._next[0] = &_mark; _mark._height = 1; }

    node_type* _M_begin() { return _mark._next[0]; }
    const node_type* _M_begin() const { return _mark._next[0]; }
    node_type* _M_end() { return &_mark; }
    const node_type* _M_end() const { return &_mark; }
    bool _M_valid_pointer(const node_type* _n) const { return _n != nullptr && _n != _M_end(); }
    // return _x < _y
    bool _M_key_compare(const key_type& _x, const key_type& _y) const { return _m_key_compare(_x, _y); }

    iterator _M_lower_bound(node_type* _x, node_type* _y, const key_type& _k);
    const_iterator _M_lower_bound(const node_type* _x, const node_type* _y, const key_type& _k) const;
    iterator _M_upper_bound(node_type* _x, node_type* _y, const key_type& _k);
    const_iterator _M_upper_bound(const node_type* _x, const node_type* _y, const key_type& _k) const;

    /**
     * @brief find the shortest path to precessor node of %_k.
     * @returns dirty list need to update. %return[0] in the main list.
     * @details 
     *   the size of %return equals %_M_current_height().
     *   contains nodes from %_makr, main list and each sub list.
     *   // the following graph as an example.
     *   _k = 6, and nodes with asterisk are return values.
     *   /// === skip list === ///
     *        begin                                      end
     *   ┌─┐                                             ┌─┐
     *    3          ┌→┐*                       
     *    2           → *                    ┌→┐($)           (sub lists)
     *    1           →          ┌→┐*         → ($)        
     *    0    ┌→┐    →    ┌→┐    → *  ┌→┐    →    ┌→┐        (main list)
     *   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘
     *  _mark   1     3     5     5     7     8     9   _mark
    */
    map_type* _M_dirty_list_prek(const key_type& _k);

    // @brief unique_insert
    std::pair<iterator, bool> _M_insert(const value_type& _v, asp::true_type);
    // @brief multi_insert
    iterator _M_insert(const value_type& _v, asp::false_type);
    size_type _M_erase(const key_type& _k);

    size_type _M_current_height() const { return _mark._height; }
    void _M_set_node_height(node_type* const _x, size_type _ht);

private:
    void _M_insert_aux(map_type* _dirty_list, size_type _n, node_type* _x);
    node_type* _M_erase_aux(map_type* _dirty_list, size_type _n, node_type* const _s);
    size_type _M_random_height() const;
};

/// skip_list private implement
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_insert_aux(map_type* _dirty_list, size_type _n, node_type* _x) -> void {
    node_type* const _s = _dirty_list[0];
    size_type _r_level = _M_random_height();
    // _x->_next = this->_M_allocate_map(_r_level);
    if (node_type* _s_next = _s->_M_next()) {
        _s_next->_prev = _x;
    }
    _M_set_node_height(_x, _r_level);
    for (int _i = 0; _i < std::min(_r_level, _n); ++_i) {
        _x->_next[_i] = _dirty_list[_i]->_next[_i];
        _dirty_list[_i]->_next[_i] = _x;
    }
    _x->_prev = _s;
    if (_r_level > _n) {
        for (int _i = _n; _i < _r_level; ++_i) {
            _x->_next[_i] = _M_end();
            _mark._next[_i] = _x;
        }
        _mark._height = _r_level;
    }
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_erase_aux(map_type* _dirty_list, size_type _n, node_type* const _s) -> node_type* {
    node_type* const _p = _dirty_list[0];
    node_type* const _r = _dirty_list[0]->_M_next();
    node_type* _rs = _s;
    for (int _i = 0; _i < _n && _rs != _p; ++_i) {
        while (_i >= _rs->_height) {
            _rs = _rs->_prev;
        }
        _dirty_list[_i]->_next[_i] = _rs->_next[_i];
    }
    while (_M_current_height() > 1 && !_M_valid_pointer(_mark._next[_M_current_height() - 1])) {
        --_mark._height;
        _mark._next[_mark._height] = nullptr;
    }
    _s->_next[0]->_prev = _dirty_list[0];
    _s->_next[0] = nullptr;
    return _r;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_random_height() const -> size_type {
    size_type _height = 1;
    while (asp::rand_float() < _S_height_prob && _height < _S_max_height) {
        ++_height;
    }
    return _height;
};

/// skip_list protected implement
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_lower_bound(node_type* _x, node_type* _y, const key_type& _k) -> iterator {
    for (int _i = _M_current_height() - 1; _i >= 0; --_i) {
        while (_M_valid_pointer(_x->_M_next(_i))) {
            node_type* _n = _x->_M_next(_i);
            if (!_M_key_compare(_S_key(_n), _k)) { // _k <= _n
                break;
            }
            _x = _n;
        }
    }
    node_type* _n = _x->_M_next();
    if (_M_valid_pointer(_n) && !_M_key_compare(_k, _S_key(_n))) {
        return iterator(_n);
    }
    else {
        return iterator(_y);
    }
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_lower_bound(const node_type* _x, const node_type* _y, const key_type& _k) const -> const_iterator {
    for (int _i = _M_current_height() - 1; _i >= 0; --_i) {
        while (_M_valid_pointer(_x->_M_next(_i))) {
            node_type* _n = _x->_M_next(_i);
            if (!_M_key_compare(_S_key(_n), _k)) { // _k <= _n
                break;
            }
            _x = _n;
        }
    }
    node_type* _n = _x->_M_next();
    if (_M_valid_pointer(_n) && !_M_key_compare(_k, _S_key(_n))) {
        return const_iterator(_n);
    }
    else {
        return const_iterator(_y);
    }
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_upper_bound(node_type* _x, node_type* _y, const key_type& _k) -> iterator {
    for (int _i = _M_current_height() - 1; _i >= 0; --_i) {
        while (_M_valid_pointer(_x->_M_next(_i))) {
            node_type* _n = _x->_M_next(_i);
            if (_M_key_compare(_k, _S_key(_n))) { // _k < _n
                break;
            }
            _x = _n;
        }
    }
    node_type* _n = _x->_M_next();
    if (_M_valid_pointer(_n) && !_M_key_compare(_k, _S_key(_n))) {
        return iterator(_n);
    }
    else {
        return iterator(_y);
    }
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_upper_bound(const node_type* _x, const node_type* _y, const key_type& _k) const -> const_iterator {
    for (int _i = _M_current_height() - 1; _i >= 0; --_i) {
        while (_M_valid_pointer(_x->_M_next(_i))) {
            node_type* _n = _x->_M_next(_i);
            if (_M_key_compare(_k, _S_key(_n))) { // _k < _n
                break;
            }
            _x = _n;
        }
    }
    node_type* _n = _x->_M_next();
    if (_M_valid_pointer(_n) && !_M_key_compare(_k, _S_key(_n))) {
        return const_iterator(_n);
    }
    else {
        return const_iterator(_y);
    }
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_dirty_list_prek(const key_type& _k) -> map_type* {
    map_type* _ret = this->_M_allocate_map(_M_current_height());
    difference_type _cnt = 0;
    node_type* _x = &_mark;
    for (int _i = _M_current_height() - 1; _i >= 0; --_i) {
        while (_M_valid_pointer(_x->_M_next(_i))) {
            node_type* _n = _x->_M_next(_i);
            if (!_M_key_compare(_S_key(_n), _k)) {
                break;
            }
            _x = _n;
        }
        _ret[_i] = _x;
    }
    return _ret;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_insert(const value_type& _v, asp::true_type) -> std::pair<iterator, bool> {
    size_type _old_height = _M_current_height();
    map_type* _res = this->_M_dirty_list_prek(_S_key(_v));

    const node_type* _bottom_node = _res[0];
    if (_bottom_node != nullptr) {
        auto _bottom_next = _bottom_node->_M_next();
        if (_M_valid_pointer(_bottom_next)) {
            if (!_M_key_compare(_S_key(_v), _S_key(_bottom_next))) {
                this->_M_deallocate_map(_res, _old_height);
                return std::make_pair(iterator(_bottom_next), false);
            }
        }
    }

    node_type* _x = this->_M_allocate_node(_v);
    _M_insert_aux(_res, _old_height, _x);
    ++_m_element_count;
    this->_M_deallocate_map(_res, _old_height);
    return std::make_pair(iterator(_x), true);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_insert(const value_type& _v, asp::false_type) -> iterator {
    size_type _old_height = _M_current_height();
    map_type* _res = this->_M_dirty_list_prek(_S_key(_v));
    node_type* _x = this->_M_allocate_node(_v);
    _M_insert_aux(_res, _old_height, _x);
    ++_m_element_count;
    this->_M_deallocate_map(_res, _old_height);
    return iterator(_x);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_erase(const key_type& _k) -> size_type {
    size_type _old_height = _M_current_height();
    map_type* _res = this->_M_dirty_list_prek(_k);

    node_type* _s = _res[0];
    if (_s == nullptr) { this->_M_deallocate_map(_res, _old_height); return 0; }
    // node_type* _s = _bottom_node;

    size_type _cnt = 0;
    while (_M_valid_pointer(_s->_M_next()) && !_M_key_compare(_k, _S_key(_s->_M_next()))) {
        ++_cnt;
        _s = _s->_M_next();
    }
    if (_cnt > 0) {
        node_type* _p = _M_erase_aux(_res, _old_height, _s);
        while (_p != nullptr) {
            node_type* _tmp = _p->_M_next();
            this->_M_deallocate_node(_p);
            _p = _tmp;
        }
    }
    _m_element_count -= _cnt;
    this->_M_deallocate_map(_res, _old_height);
    return _cnt;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_set_node_height(node_type* const _x, size_type _ht) -> void {
    if (_x->_next != nullptr) {
        this->_M_deallocate_map(_x->_next, _x->_height);
    }
    _x->_next = this->_M_allocate_map(_ht);
    _x->_height = _ht;
};

/// skip_list public implement
// template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
// skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::skip_list() {

// }
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::~skip_list() {
    clear();
    this->_M_deallocate_map(_mark._next, _S_max_height);
}

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::find(const key_type& _k) const -> const_iterator {
    const_iterator _j = _M_lower_bound(_M_begin(), _M_end(), _k);
    return (_j == cend() || _M_key_compare(_k, _S_key(_j._ptr))) ? cend() : _j;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::count(const key_type& _k) const
-> size_type {
    const_iterator _first(_M_lower_bound(_M_begin(), _M_end(), _k));
    const_iterator _last(_M_upper_bound(_M_begin(), _M_end(), _k));
    return asp::distance(_first, _last);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::clear()
-> void {
    if (empty()) return;
    node_type* _prev = nullptr;
    node_type* _p = _mark._M_next();
    for (; _p != nullptr && _p != &_mark;) {
        _prev = _p;
        _p = _p->_M_next();
        this->_M_deallocate_node(_prev);
    }
    _M_init_mark();
    _m_element_count = 0;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::insert(const value_type& _v)
-> ireturn_type {
    return this->_M_insert(_v, asp::bool_t<_UniqueKey>());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::erase(const key_type& _k)
-> size_type {
    return this->_M_erase(_k);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::check() const -> int {
    /**
     * @returns 0 = normal;
     * 1 = duplicate value in unique container;
     * 2 = not in order (store in order could promise that the same value(s) are stored adjacent);
     * 3 = empty height list (%_mark._next[_i] point to %_mark);
     * 4 = the number of traversed nodes is not equal to %_element_count;
     * 5 = node's height is less than current sublist.
    */
    size_type _count = 0;
    std::unordered_set<key_type> _uset;
    const bool _unique = _UniqueKey;
    asp::decay_t<key_type> _last_value;
    size_type _sl_height = this->_M_current_height();
    for (int _i = 0; _i < _sl_height; ++_i) {
        node_type* _p = _mark._next[_i];
        bool _first_elem = true;
        while (_M_valid_pointer(_p)) {
            if (_p->_height < _i) {
                return 5;
            }
            const key_type _k = _S_key(_p);
            ++_count;
            if (_uset.count(_k)) {
                if (_unique) {
                    return 1;
                }
            }
            if (!_first_elem) {
                if (_M_key_compare(_k, _last_value)) {
                    return 2;
                }
            }
            _uset.insert(_k);
            _last_value = _k;
            _p = _p->_next[_i];
            _first_elem = false;
        }
        if (_i != 0 && _first_elem) {
            return 3;
        }
        if (_i == 0 && _count != _m_element_count) {
            return 4;
        }
        _uset.clear();
    }
    return 0;
};


/// output implement
template <typename _K, typename _V, typename _EK, bool _UK, typename _C, typename _A>
std::ostream& operator<<(std::ostream& os, const skip_list<_K, _V, _EK, _UK, _C, _A>& _sl) {
    os << '[';
    for (auto p = _sl.cbegin(); p != _sl.cend();) {
        os << p;
        if (_sl._log_height && p) { os << "(" << p._ptr->_height << ")";}
        if (++p != _sl.cend()) {
            os << ", ";
        }
    }
    os << ']';
    return os;
}

};

#endif  // _ASP_SKIP_LIST_HPP_