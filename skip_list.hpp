#ifndef _ASP_SKIP_LIST_HPP_
#define _ASP_SKIP_LIST_HPP_

#include "basic_param.hpp"
#include "skip_list_node.hpp"
#include "associative_container_aux.hpp"
#include "random.hpp"

#include <cstring>

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
 *    3          ┌→┐                     
 *    2           →                      ┌→┐        
 *    1           →          ┌→┐          →         
 *    0    ┌→┐    →    ┌→┐    →    ┌→┐    →    ┌→┐  
 *   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘   └─┘
 *  _mark   1     3     5     5     7     8     9   _mark
 * // skip list
 * 
 * 1. Node height monotonically decreases ↓ when looking up.
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
    skip_list() { _M_init_mark(); }
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
    std::pair<iterator, iterator> equal_range(const key_type& _k);
    std::pair<const_iterator, const_iterator> equal_range(const key_type& _k) const;

protected:
    static constexpr const size_type _S_max_height = 8;
    static constexpr const double _S_height_prob = 0.25;

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
     * @brief find a suitable node to insert.
     * @returns dirty list need to update. %return[0] is the bottom node.
     * @details iterative lookup for a suitble lead node to insert.
    */
    map_type* _M_insert_position(const key_type& _k);

    // @brief unique_insert
    std::pair<iterator, bool> _M_insert(const value_type& _v, asp::true_type);
    // @brief multi_insert
    iterator _M_insert(const value_type& _v, asp::false_type);
    size_type _M_erase(const_iterator _p);
    size_type _M_erase(const_iterator _first, const_iterator _last);

    size_type _M_current_height() const { return _mark._height; }

private:
    void _M_insert_aux(map_type* _dirty_list, size_type _n, node_type* _x);
    node_type* _M_erase_aux(node_type* const _s);
    size_type _M_random_height() const;
};

/// skip_list private implement
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_insert_aux(map_type* _dirty_list, size_type _n, node_type* _x) -> void {
    node_type* const _s = _dirty_list[0];
    size_type _r_level = _M_random_height();
    // _x->_next = this->_M_allocate_map(_r_level);
    _x->_height = _r_level;
    for (int _i = 0; _i < _n; ++_i) {
        _x->_next[_i] = _dirty_list[_i]->_next[_i];
        _dirty_list[_i]->_next[_i] = _x;
    }
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
_M_erase_aux(node_type* const _s) -> node_type* {};
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
_M_lower_bound(node_type* _x, node_type* _y, const key_type& _k) -> iterator {};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_lower_bound(const node_type* _x, const node_type* _y, const key_type& _k) const -> const_iterator {};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_upper_bound(node_type* _x, node_type* _y, const key_type& _k) -> iterator {};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_upper_bound(const node_type* _x, const node_type* _y, const key_type& _k) const -> const_iterator {};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_insert_position(const key_type& _k) -> map_type* {
    map_type* _ret = this->_M_allocate_map(_S_max_height);
    difference_type _cnt = 0;
    node_type* _x = &_mark;
    bool _comp_res;
    for (int _i = _M_current_height() - 1; _i >= 0; --_i) {
        node_type* _n = _x->_M_next(_i);
        while (_M_valid_pointer(_n) && _M_key_compare(_S_key(_n), _k)) {
            _x = _n;
        }
        _ret[_cnt++] = _x;
    }
    return _ret;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_insert(const value_type& _v, asp::true_type) -> std::pair<iterator, bool> {
    size_type _old_height = _M_current_height();
    map_type* _res = this->_M_insert_position(_S_key(_v));

    const node_type* _bottom_node = _res[0];
    if (_M_valid_pointer(_bottom_node)) {
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
    this->_M_deallocate_map(_res, _old_height);
    return std::make_pair(iterator(_x), true);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_insert(const value_type& _v, asp::false_type) -> iterator {
    size_type _old_height = _M_current_height();
    map_type* _res = this->_M_insert_position(_S_key(_v));
    node_type* _x = this->_M_allocate_node(_v);
    _M_insert_aux(_res, _old_height, _x);
    this->_M_deallocate_map(_res, _old_height);
    return iterator(_x);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_erase(const_iterator _p) -> size_type {};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::
_M_erase(const_iterator _first, const_iterator _last) -> size_type {};

/// skip_list public implement
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::find(const key_type& _k) const -> const_iterator {
    const_iterator _j = _M_lower_bound(_M_begin(), _M_end(), _k);
    return (_j == cend() || _M_key_compare(_k, _S_key(_j._ptr))) ? cend() : _j;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::count(const key_type& _k) const
-> size_type {
    std::pair<const_iterator, const_iterator> _res = equal_range(_k);
    const size_type _n = asp::distance(_res.first, _res.second);
    return _n;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::clear()
-> void {
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::insert(const value_type& _v)
-> ireturn_type {
    return this->_M_insert(_v, asp::bool_t<_UniqueKey>());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
skip_list<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::erase(const key_type& _k)
-> size_type {
    std::pair<const_iterator, const_iterator> _p = equal_range(_k);
    return this->_M_erase(_p.first, _p.second);
};


};

#endif  // _ASP_SKIP_LIST_HPP_