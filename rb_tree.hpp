#ifndef _RB_TREE_HPP_
#define _RB_TREE_HPP_

#include "tree_node.hpp"
#include "type_traits.hpp"

#include "memory.hpp"
// #include <memory>

namespace asp {

enum _Rb_tree_color { _S_red = false, _S_black = true };
template <typename _Tp> struct rb_tree_node;
template <typename _Tp> struct rb_tree_header;
template <typename _Value, typename _Alloc> struct rb_tree_alloc;

template <typename _Tp> struct rb_tree_iterator;
template <typename _Tp> struct rb_tree_const_iterator;

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> class rb_tree;

template <typename _Tp> struct rb_tree_node : public bitree_node<_Tp> {
    typedef bitree_node<_Tp> base;
    typedef rb_tree_node<_Tp> self;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using reference = typename base::reference;
    _Rb_tree_color _color;

    using base::left_rotate;
    using base::right_rotate;

    static self* _S_minimum(self* _x) {
        while (_x->_left != nullptr) {
            _x = _x->_left;
        }
        return _x;
    }
    static self* _S_maximum(self* _x) {
        while (_x->_right != nullptr) {
            _x = _x->_right;
        }
        return _x;
    }

};

/**
 * @brief helper type to manage default initialization of node
 * @details %_header manage the basic info of @rb_tree
 *   %_header._parent = real root node of @rb_tree
 *   %_header = end of traversation
 *   %_header._left = minium node (the leftmost node in tree)
 *   %_header._right = maxium node (the rightmost node in tree)
*/
template <typename _Tp> struct rb_tree_header : public bitree_header<_Tp> {
    typedef bitree_header<_Tp> base;
    typedef rb_tree_header<_Tp> self;
    typedef rb_tree_node<_Tp> _Node;
    typedef typename _Node::value_type value_type;
    rb_tree_node<_Tp> _header;  // bitree_header::_header override
    using base::_node_count;

    rb_tree_header() : base() {
        _header._color = _S_red;
        reset();
    }
    rb_tree_header(rb_tree_header&& _x) {
        if (_x._header._parent != nullptr) {
            move_data(_x);
        }
        else {
            _header._color = _S_red;
            reset();
        }
    }

    void move_data(rb_tree_header& _from) {
        _header._color = _from._header._color;
        _header._parent = _from._header._parent;
        _header._left = _from._header._left;
        _header._right = _from._header._right;
        // _header._parent->
        this->_node_count = _from._node_count;
        _from.reset();
    }

    void reset() {
        base::reset();
    }
};


template <typename _Value, typename _Alloc> struct rb_tree_alloc
: public _Alloc {
    typedef rb_tree_node<_Value> node_type;

    typedef _Alloc elt_allocator_type;
    typedef std::allocator_traits<elt_allocator_type> elt_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<node_type> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_alloc_traits;

    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type*>(this); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<const elt_allocator_type*>(this); }
    node_allocator_type _M_get_node_allocator() const { return node_allocator_type(_M_get_elt_allocator()); }

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
};

template <typename _Tp> struct rb_tree_iterator {
    typedef asp::bidirectional_iterator_tag iterator_category;
    typedef rb_tree_node<_Tp> node_type;
    typedef typename node_type::value_type value_type;
    typedef rb_tree_iterator<_Tp> self;

    node_type* _ptr = nullptr;

    rb_tree_iterator() = default;
    rb_tree_iterator(node_type* _x) : _ptr(_x) {}
    value_type& operator*() const { return _ptr->val(); }
    value_type* operator->() const { return _ptr->valptr(); }
    self& operator++() { _ptr = __bitree__::bitree_node_increase(_ptr); return *this; }
    self operator++(int) { self _ret = *this; _ptr = __bitree__::bitree_node_increase(_ptr); return _ret; }
    self& operator--() { _ptr = __bitree__::bitree_node_decrease(_ptr); return *this; }
    self operator--(int) { self _ret = *this; _ptr = __bitree__::bitree_node_decrease(_ptr); return _ret; }
    friend bool operator==(const self& _x, const self& _y) { return _x._ptr == _y._ptr; }
    friend bool operator!=(const self& _x, const self& _y) { return _x._ptr != _y._ptr; }
};
template <typename _Tp> struct rb_tree_const_iterator {
    typedef asp::bidirectional_iterator_tag iterator_category;
    typedef rb_tree_node<_Tp> node_type;
    typedef typename node_type::value_type value_type;
    typedef rb_tree_const_iterator<_Tp> self;
    typedef rb_tree_iterator<_Tp> iterator;

    const node_type* _ptr = nullptr;

    rb_tree_const_iterator() = default;
    rb_tree_const_iterator(const node_type* _x) : _ptr(_x) {}
    
    const value_type& operator*() const { return _ptr->val(); }
    const value_type* operator->() const { return _ptr->valptr(); }
    iterator _const_cast() const { return iterator(const_cast<node_type*>(_ptr)); }
    self& operator++() { _ptr = __bitree__::bitree_node_increase(_ptr); return *this; }
    self operator++(int) { self _ret = *this; _ptr = __bitree__::bitree_node_increase(_ptr); return _ret; }
    self& operator--() { _ptr = __bitree__::bitree_node_decrease(_ptr); return *this; }
    self operator--(int) { self _ret = *this; _ptr = __bitree__::bitree_node_decrease(_ptr); return _ret; }
    friend bool operator==(const self& _x, const self& _y) { return _x._ptr == _y._ptr; }
    friend bool operator!=(const self& _x, const self& _y) { return _x._ptr != _y._ptr; }
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp = std::less<_Key>, typename _Alloc = std::allocator<_Value>>
class rb_tree : public rb_tree_alloc<_Value, _Alloc> {
public:
    typedef rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc> self;
    typedef rb_tree_alloc<_Value, _Alloc> base;
    typedef rb_tree_alloc<_Value, _Alloc> rbt_alloc;
    typedef typename rbt_alloc::elt_allocator_type elt_allocator_type;
    typedef typename rbt_alloc::elt_alloc_traits elt_alloc_traits;
    typedef typename rbt_alloc::node_allocator_type node_allocator_type;
    typedef typename rbt_alloc::node_alloc_traits node_alloc_traits;

    typedef _Key key_type;
    typedef typename base::node_type node_type;
    typedef const node_type const_node_type;
    typedef typename node_type::value_type value_type;

    typedef rb_tree_iterator<value_type> iterator;
    typedef rb_tree_const_iterator<value_type> const_iterator;

    typedef asp::conditional_t<_UniqueKey, std::pair<iterator, bool>, iterator> ireturn_type;

    rb_tree_header<_Value> _m_impl;
    _ExtKey _m_extract_key;
    _Comp _m_key_compare;


    static const value_type& _S_value(const_node_type* _x) { return _x->val(); }
    static const key_type& _S_key(const_node_type* _x) { return _ExtKey()(_x->val()); }

public:
    iterator begin() { return iterator(_M_leftmost()); }
    const_iterator cbegin() const { return const_iterator(_M_leftmost()); }
    iterator end() { return iterator(_M_rightmost()); }
    const_iterator cend() const { return const_iterator(_M_rightmost()); }
    size_type size() const { return _m_impl._node_count; }
    bool empty() const { return _m_impl._node_count == 0; }

    iterator find(const key_type& _k);
    const_iterator find(const key_type& _k) const;
    size_type count(const key_type& _k) const;
    void clear();
    ireturn_type insert(const value_type& _v);
    size_type erase(const key_type& _k);

    iterator lower_bound(const key_type& _k) { return _M_lower_bound(_M_begin(), _M_end(), _k); }
    const_iterator lower_bound(const key_type& _k) const { return _M_lower_bound(_M_begin(), _M_end(), _k); }
    iterator lower_bound(const key_type& _k) { return _M_upper_bound(_M_begin(), _M_end(), _k); }
    const_iterator lower_bound(const key_type& _k) const { return _M_upper_bound(_M_begin(), _M_end(), _k); }
    std::pair<iterator, iterator> equal_range(const key_type& _k);
    std::pair<const_iterator, const_iterator> equal_range(const key_type& _k) const;

protected:
    node_type* _M_root() { return _m_impl._header._parent; }
    const_node_type* _M_root() const { return _m_impl._header._parent; }
    node_type* _M_leftmost() { return _m_impl._header._left; }
    const_node_type* _M_leftmost() const { return _m_impl._header._left; }
    node_type* _M_rightmost() { return _m_impl._header._right; }
    const_node_type* _M_rightmost() const { return _m_impl._header._right; }
    node_type* _M_begin() { return _m_impl._header._parent; }
    const_node_type* _M_begin() const { return _m_impl._header._parent; }
    node_type* _M_end() { return &_m_impl._header; }
    const_node_type* _M_end() const { return &_m_impl._header; }

    // return _x < _y;
    bool _M_key_compare(const key_type& _x, const key_type& _y) const { return _m_key_compare(_x, _y); }
    /**
     * @brief find the greatest node (_i) less than _k in range [_x, _y), _S_key(_i) < _k
     * @details not exists _i in [_x, _y), _S_key(_i) < _S_key(_j) < _k
    */
    iterator _M_lower_bound(node_type* _x, node_type* _y, const key_type& _k);
    const_iterator _M_lower_bound(const node_type* _x, const node_type* _y, const key_type& _k) const;
    /**
     * @brief find the least node (_i) greater than _k in range [_x, _y), _k < _S_key(_i)
     * @details not exists _i in [_x, _y), _k < _S_key(_j) < _S_key(_i)
    */
    iterator _M_upper_bound(node_type* _x, node_type* _y, const key_type& _k);
    const_iterator _M_upper_bound(const node_type* _x, const node_type* _y, const key_type& _k) const;

    std::pair<iterator, bool> _M_insert(const value_type& _v, asp::true_type);
    iterator _M_insert(const value_type& _v, asp::false_type);
    iterator _M_insert_unique(const value_type& _v);
    iterator _M_insert_multi(const value_type& _v);
    size_type _M_erase(const key_type& _k, asp::true_type);
    size_type _M_erase(const key_type& _k, asp::false_type);
};


/// rb_tree protected implement
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_lower_bound(node_type* _x, node_type* _y, const key_type& _k)
-> iterator {
    while (_x != nullptr) {
        if (_M_key_compare(_S_key(_x), _k)) {
            _x = _x->_right;
        }
        else {
            _y = _x;
            _x = _x->_left;
        }
    }
    return iterator(_y);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_lower_bound(const node_type* _x, const node_type* _y, const key_type& _k) const
-> const_iterator {
    while (_x != nullptr) {
        if (_M_key_compare(_S_key(_x), _k)) {
            _x = _x->_right;
        }
        else {
            _y = _x;
            _x = _x->_left;
        }
    }
    return const_iterator(_y);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_upper_bound(node_type* _x, node_type* _y, const key_type& _k)
-> iterator {
    while (_x != nullptr) {
        if (_M_key_compare(_k, _S_key(_x))) {
            _y = _x;
            _x = _x->_left;
        }
        else {
            _x = _x->_right;
        }
    }
    return iterator(_y);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_upper_bound(const node_type* _x, const node_type* _y, const key_type& _k) const
-> const_iterator {
    while (_x != nullptr) {
        if (_M_key_compare(_k, _S_key(_x))) {
            _y = _x;
            _x = _x->_left;
        }
        else {
            _x = _x->_right;
        }
    }
    return const_iterator(_y);
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert(const value_type& _v, asp::true_type) -> std::pair<iterator, bool> {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert(const value_type& _v, asp::false_type) -> iterator {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert_unique(const value_type& _v) -> iterator {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert_multi(const value_type& _v) -> iterator {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_erase(const key_type& _k, asp::true_type) -> size_type {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_erase(const key_type& _k, asp::false_type) -> size_type {

};


/// rb_tree public implement
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::find(const key_type& _k)
-> iterator {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::find(const key_type& _k) const
-> const_iterator {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::count(const key_type& _k) const
-> size_type {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::clear()
-> void {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::insert(const value_type& _v)
-> ireturn_type {
    return this->_M_insert(_v, asp::bool_t<_UniqueKey>());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::erase(const key_type& _k)
-> size_type {
    return this->_M_erase(_k, asp::bool_t<_UniqueKey>());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::equal_range(const key_type& _k)
-> std::pair<iterator, iterator> {
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::equal_range(const key_type& _k) const
-> std::pair<const_iterator, const_iterator> {
};
};

#endif