#ifndef _RB_TREE_HPP_
#define _RB_TREE_HPP_

#include "tree_node.hpp"

#include "iterator.hpp"
#include "type_traits.hpp"
#include "associative_container_aux.hpp"

#include "memory.hpp"
// #include <memory>

namespace asp {

enum _Rb_tree_color { _S_red = false, _S_black = true };
template <typename _Tp> struct rb_tree_node;
template <typename _Tp> struct rb_tree_header;
template <typename _Value, typename _Alloc> struct rb_tree_alloc;

template <typename _Tp> struct rb_tree_iterator;
template <typename _Tp> struct rb_tree_const_iterator;

/**
 * @brief red black tree
 * @details
 *   here are 5 rules for rb_tree, which maintain the balance of rb_tree.
 *     rule 1: the color of each node (inner node and leaf node) is red or black.
 *     rule 2: the color of root node is black.
 *     rule 3: we regard nullptr as black node.
 *     rule 4: all red node's children must be black.
 *     rule 5: all paths from a node to its descendants (until nullptr) contain the same number of black nodes.
 * @def
 *   black height: the number of black nodes in the path from the given node to its descendants (until nullptr)
 *   relationship: indicates whether the child node is left or right child of its parent.
*/
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> class rb_tree;

namespace __rb_tree__ {
template <typename _Tp> bool _S_as_black_node(const rb_tree_node<_Tp>* _x);

/**
 * @brief check the rb_tree's 5 rules
*/
template <typename _Tp> int _S_check(const rb_tree_node<_Tp>* _header);
/**
 * @brief black height of subtree(_s)
 * @return -1 : error in black height, -2 : broken 4th rule
 * */
template <typename _Tp> int _S_black_height(const rb_tree_node<_Tp>* _s, int _bh = 0);
};

template <typename _Tp> struct rb_tree_node : public bitree_node<_Tp> {
    typedef bitree_node<_Tp> base;
    typedef rb_tree_node<_Tp> self;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using reference = typename base::reference;

    rb_tree_node() : base() {}
    rb_tree_node(const self& _s) : base(_s), _color(_s._color) {}
    rb_tree_node(self&& _s) : base(_s), _color(_s._color) {}
    rb_tree_node(const value_type& _v) : base(_v) {}
    template <typename... _Args> rb_tree_node(_Args&&... _args) : base(std::forward<_Args>(_args)...) {}
    virtual ~rb_tree_node() {}

    _Rb_tree_color _color;
    self* _parent = nullptr;
    self* _left = nullptr;
    self* _right = nullptr;

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

    _Rb_tree_color _M_reverse_color() {
        if (_color == _Rb_tree_color::_S_red) {
            _color = _Rb_tree_color::_S_black;
        }
        else {
            _color = _Rb_tree_color::_S_red;
        }
        return _color;
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
        _header._parent = nullptr;
        _header._left = &_header;
        _header._right = &_header;
        _header._color = _S_red;
        _node_count = 0;
    }
};


template <typename _Value, typename _Alloc> struct rb_tree_alloc
: public _Alloc {
    typedef rb_tree_node<_Value> node_type;
    typedef typename node_type::base node_type_base;

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
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef asp::difference_type difference_type;
    typedef rb_tree_iterator<_Tp> self;

    node_type* _ptr = nullptr;

    rb_tree_iterator() = default;
    rb_tree_iterator(node_type* _x) : _ptr(_x) {}
    value_type& operator*() const { return _ptr->val(); }
    value_type* operator->() const { return _ptr->valptr(); }
    self& operator++() { _ptr = __bitree__::_S_bitree_node_increase(_ptr); return *this; }
    self operator++(int) { self _ret = *this; _ptr = __bitree__::_S_bitree_node_increase(_ptr); return _ret; }
    self& operator--() { _ptr = __bitree__::_S_bitree_node_decrease(_ptr); return *this; }
    self operator--(int) { self _ret = *this; _ptr = __bitree__::_S_bitree_node_decrease(_ptr); return _ret; }
    operator bool() const { return _ptr != nullptr; }
    friend bool operator==(const self& _x, const self& _y) { return _x._ptr == _y._ptr; }
    friend bool operator!=(const self& _x, const self& _y) { return _x._ptr != _y._ptr; }
    template <typename _T> friend std::ostream& operator<<(std::ostream& os, const rb_tree_iterator<_T>& _r);
};
template <typename _Tp> struct rb_tree_const_iterator {
    typedef asp::bidirectional_iterator_tag iterator_category;
    typedef rb_tree_node<_Tp> node_type;
    typedef typename node_type::value_type value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef asp::difference_type difference_type;
    typedef rb_tree_const_iterator<_Tp> self;
    typedef rb_tree_iterator<_Tp> iterator;

    const node_type* _ptr = nullptr;

    rb_tree_const_iterator() = default;
    rb_tree_const_iterator(const node_type* _x) : _ptr(_x) {}
    rb_tree_const_iterator(const iterator& _i) : _ptr(_i._ptr) {}
    
    const value_type& operator*() const { return _ptr->val(); }
    const value_type* operator->() const { return _ptr->valptr(); }
    iterator _const_cast() const { return iterator(const_cast<node_type*>(_ptr)); }
    self& operator++() { _ptr = __bitree__::_S_bitree_node_increase(_ptr); return *this; }
    self operator++(int) { self _ret = *this; _ptr = __bitree__::_S_bitree_node_increase(_ptr); return _ret; }
    self& operator--() { _ptr = __bitree__::_S_bitree_node_decrease(_ptr); return *this; }
    self operator--(int) { self _ret = *this; _ptr = __bitree__::_S_bitree_node_decrease(_ptr); return _ret; }
    operator bool() const { return _ptr != nullptr; }
    friend bool operator==(const self& _x, const self& _y) { return _x._ptr == _y._ptr; }
    friend bool operator!=(const self& _x, const self& _y) { return _x._ptr != _y._ptr; }
    template <typename _T> friend std::ostream& operator<<(std::ostream& os, const rb_tree_const_iterator<_T>& _r);
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
    typedef _Comp key_compare;
    typedef typename base::node_type node_type;
    typedef const node_type const_node_type;
    typedef typename base::node_type_base node_type_base;
    typedef const node_type_base const_node_type_base;
    
    typedef typename node_type::value_type value_type;

    typedef rb_tree_iterator<value_type> iterator;
    typedef rb_tree_const_iterator<value_type> const_iterator;

    typedef asp::conditional_t<_UniqueKey, std::pair<iterator, bool>, iterator> ireturn_type;

    typedef asso_container::type_traits<value_type, _UniqueKey> _ContainerTypeTraits;

    typedef typename _ContainerTypeTraits::insert_status insert_status;
    typedef typename _ContainerTypeTraits::ext_iterator ext_iterator;
    typedef typename _ContainerTypeTraits::ext_value ext_value;
    typedef typename _ContainerTypeTraits::mapped_type mapped_type;
    typedef _ExtKey ext_key;

    rb_tree_header<_Value> _m_impl;
    _ExtKey _m_extract_key;
    _Comp _m_key_compare;


    static const value_type& _S_value(const_node_type* _x) { return _x->val(); }
    static key_type _S_key(const_node_type* _x) { return _ExtKey()(_x->val()); }
    static key_type _S_key(const value_type& _v) { return _ExtKey()(_v); }

    template <typename _K, typename _V, typename _EK, bool _UK, typename _C, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const rb_tree<_K, _V, _EK, _UK, _C, _A>& _h);

public:
    rb_tree() = default;
    rb_tree(const self& _rbt);
    virtual ~rb_tree();
    template <typename _NodeGen> void _M_assign(const self& _rbt, const _NodeGen&);
    template <typename _NodeGen> node_type* _M_clone_tree(const node_type* _x, node_type* _y, const _NodeGen&);

    iterator begin() { return iterator(_M_leftmost()); }
    const_iterator cbegin() const { return const_iterator(_M_leftmost()); }
    iterator end() { return iterator(&_m_impl._header); }
    const_iterator cend() const { return const_iterator(&_m_impl._header); }
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
    iterator upper_bound(const key_type& _k) { return _M_upper_bound(_M_begin(), _M_end(), _k); }
    const_iterator upper_bound(const key_type& _k) const { return _M_upper_bound(_M_begin(), _M_end(), _k); }
    std::pair<iterator, iterator> equal_range(const key_type& _k);
    std::pair<const_iterator, const_iterator> equal_range(const key_type& _k) const;

    // used for test
    int check() const;

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
     * @brief find the first node (_i) \ge than _k in _x subtree. _S_key(_i) >= _k
     * @return return _y if all nodes are less than _k
    */
    iterator _M_lower_bound(node_type* _x, node_type* _y, const key_type& _k);
    const_iterator _M_lower_bound(const node_type* _x, const node_type* _y, const key_type& _k) const;
    /**
     * @brief find the first node (_i) greater than _k in range _x subtree, _k < _S_key(_i)
     * @return return _y if all nodes are \le than _k
    */
    iterator _M_upper_bound(node_type* _x, node_type* _y, const key_type& _k);
    const_iterator _M_upper_bound(const node_type* _x, const node_type* _y, const key_type& _k) const;

    /**
     * @brief find a suitable leaf node to insert.
     * @returns %second : inserted position. if nullptr, %first is the %_k position.
     * @details iterative lookup for a suitble lead node to insert.
    */
    std::pair<node_type*, node_type*> _M_insert_unique_position(const key_type& _k);
    // @brief find a suitable leaf node to insert.
    node_type* _M_insert_multi_position(const key_type& _k);

    // @brief unique_insert
    std::pair<iterator, bool> _M_insert(const value_type& _v, asp::true_type);
    // @brief multi_insert
    iterator _M_insert(const value_type& _v, asp::false_type);
    size_type _M_erase(const_iterator _p);
    size_type _M_erase(const_iterator _first, const_iterator _last);

    // erase subtree directly, without rebalancing
    void _M_erase_subtree(node_type* _s);

private:
    /**
     * @brief insert %_x as child of %_s in binary tree.
    */
    void _M_insert_rebalance(node_type* _p, node_type* _x);
    /**
     * @brief erase %_s in %_header's binary tree.
     * @return the node should be deallocated.
    */
    node_type* _M_erase_rebalance(node_type* const _s);
};

/// rb_tree private implement
/**
 * @details
 * - insertion
 *   %_x->_color = _S_red, and insert.
 * - rebalance
 *   %_x->_color == _S_red
 *   == %_x is the current node, of which color is always red. (no matter in insertion or iteration) ==
 *   case 1: %_x == %_root, just black it.
 *   case 2: %_x->_parent->_color == _S_black, just done.
 *   case 3: (%_x->_parent->_color == _S_red), divided into 2 cases: (by uncle node's color)
 *     (infer that _xpp->_color == _S_black)
 *     name uncle node as (_y), grandparent node as (_xpp)
 *     case 3.1: _y->_color == _S_red
 *       black _x->_parent and _y, red _xpp. and continue to iterate with _xpp as _x
 *     case 3.2: _y->_color == _S_black (only appear during iteration)
 *       case 3.2.1: relationship between (_x, _x->_parent) and (_x->_parent, _xpp) is different.
 *         let _x point to its parent, and rotate _x, transform into the latter case (case 3.2.2).
 *       case 3.2.2: relationship between (_x, _x->_parent) and (_x->_parent, _xpp) is identical.
 *         reverse the color of _x->_parent and _xpp, and rotate _xpp.
 * 
 *   the details for case 3.1:
 *     the color of _x and _x->_parent are all red, which breaks the 4th rule.
 *     thus, we black _x->_parent and _y, red _xpp, in order to keep the black height in subtree (_xpp as root).
 *     due to _xpp->_color is red, we may break the 4th rule (_xpp->_parent->_color may be red, too), so continue to iteration.
 *   the details for case 3.2:
 *     the current node's color is always red! the purpose of adjustment is to maintain the 4th rule.
*/
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert_rebalance(node_type* _p, node_type* _x) -> void {
    // node_type& _header = _m_impl._header;
    // node_type*& _root = _header._parent;
    node_type& _header = _m_impl._header;
    node_type*& _root = _header._parent;

    // initialization
    _x->_parent = _p;
    _x->_left = nullptr;
    _x->_right = nullptr;
    _x->_color = _S_red;

    // insert
    bool _insert_left = (
        _p == _M_end() ||
        _M_key_compare(_S_key(_x), _S_key(_p))
    );
    if (_insert_left) {
        _p->_left = _x;
        if (_p == &_header) {
            _header._parent = _x;
            _header._right = _x;
        }
        else if (_p == _header._left) {
            _header._left = _x;
        }
    }
    else {
        _p->_right = _x;
        if (_p == _header._right) {
            _header._right = _x;
        }
    }

    // rebalance
    while (_x != _root && _x->_parent->_color == _S_red) { // break in case 1 & 2
        node_type* const _xpp = _x->_parent->_parent;
        if (_x->_parent == _xpp->_left) {
            node_type* const _y = _xpp->_right; // uncle node
            if (_y != nullptr && _y->_color == _S_red) { // case 3.1
                _x->_parent->_color = _S_black;
                _y->_color = _S_black;
                _xpp->_color = _S_red;
                _x = _xpp;
            }
            else { // case 3.2
                if (_x == _x->_parent->_right) { // case 3.2.1
                    _x = _x->_parent;
                    __bitree__::_S_left_rotate(_x, &_m_impl._header);
                }
                // case 3.2.2
                _x->_parent->_color = _S_black;
                _xpp->_color = _S_red;
                __bitree__::_S_right_rotate(_xpp, &_m_impl._header);
            }
        }
        else {
            node_type* const _y = _xpp->_left; // uncle node
            if (_y != nullptr && _y->_color == _S_red) { // case 3.1
                _x->_parent->_color = _S_black;
                _y->_color = _S_black;
                _xpp->_color = _S_red;
                _x = _xpp;
            }
            else { // case 3.2
                if (_x == _x->_parent->_left) { // case 3.2.1
                    _x = _x->_parent;
                    __bitree__::_S_right_rotate(_x, &_m_impl._header);
                }
                // case 3.2.2
                _x->_parent->_color = _S_black;
                _xpp->_color = _S_red;
                __bitree__::_S_left_rotate(_xpp, &_m_impl._header);
            }
        }
    }
    _root->_color = _S_black;
};

/**
 * @details
 * - deletion:
 *   delete node as a normal binary tree.
 *   to leaf node, delete directly.
 *   to node with single child, delete and let its child take its place.
 *   to node with two child, swap it and its successor node (with less than one child), and delete.
 * - rebalance:
 *   case 1: %_y->_color == _S_red, done.
 *   // iteration cases
 *   case 2: %_x->_color == _S_red, black it and done.
 *   case 3: %_x == _root, done.
 *   case 4: (%_x != _root, %_x->_color != _S_red), divided into 4 cases:
 *     // black height of _x subtree is less than its sibling node.
 *     // suppose that %_x == _x_parent->_left, vice versa
 *     // name %_x 's sibling node as %_w
 *     case 4.1: %_w->_color == _S_red.
 *       reverse the color of %_w & %_x_parent, and left rotate the %_x_parent.
 *       (transform into case 4.2, 4.3, 4.4)
 *     case 4.2: %_w->_left->_color == _S_black, %_w->_right->_color == _S_black.
 *       red %_w, and iterate with _x_parent as _x
 *     case 4.3: %_w->_left->_color == _S_red, %_w->_right->_color == _S_black.
 *       red %_w, black %_w->_left, and right rotate %_w
 *       (transform into case 4.4)
 *     case 4.4: %_w->_left->_color == _S_black, %_w->_right->_color == _S_red.
 *       %_w->_color = _x_parent->_color, red %_w->_right, black %_x_parent
 *       and left rotate %_x_parent.
 *       notice that, the black height of _x_parent subtree hasn't changed, so break directly.
*/
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_erase_rebalance(node_type* const _s) -> node_type* {
    node_type& _header = _m_impl._header;
    node_type*& _root = _header._parent;
    node_type*& _leftmost = _header._left;
    node_type*& _rightmost = _header._right;
    node_type* _y = _s; // node to delete
    node_type* _x = nullptr; // the child of %_y
    node_type* _x_parent = nullptr; // the parent of %_x (not %_y)
    if (_s->_left == nullptr) {
        _x = _s->_right;
    }
    else {
        if (_s->_right == nullptr) {
            _x = _s->_left;
        }
        else {
            _y = __bitree__::_S_bitree_node_increase(_y); // successor node of %_s
            _x = _y->_right;
        }
    }
    // %_x may be nullptr

    // relink and separate out %_s
    if (_y != _s) { // swap _s and its successor node. replace _s with _y, and _y = _s
        // cope _s->_left. // _y must in the right subtree of _s, _y->_left == nullptr
        _s->_left->_parent = _y;
        _y->_left = _s->_left;

        if (_y != _s->_right) { // cope _s->_right
            _x_parent = _y->_parent;
            if (_x != nullptr) _x->_parent = _y->_parent;
            _y->_parent->_left = _x; // %_y must be a left child.
            _y->_right = _s->_right;
            _s->_right->_parent = _y;
        }
        else {
            _x_parent = _y;
        }

        // cope _s->_parent
        if (_s == _root) {
            _root = _y;
        }
        else if (_s->_parent->_left == _s) {
            _s->_parent->_left = _y;
        }
        else {
            _s->_parent->_right = _y;
        }
        _y->_parent = _s->_parent;

        std::swap(_y->_color, _s->_color);
        _y = _s;
    }
    else { // _y == _s, _s owns less than one child.
        _x_parent = _s->_parent;
        if (_x != nullptr) {
            _x->_parent = _s->_parent;
        }

        // cope %_s->_parent
        if (_s == _root) {
            _root = _x;
        }
        else {
            if (_s->_parent->_left == _s) {
                _s->_parent->_left = _x;
            }
            else {
                _s->_parent->_right = _x;
            }
        }

        // update left/right most
        if (_leftmost == _s) {
            if (_s->_right == nullptr) {
                _leftmost = _s->_parent;
            }
            else {
                _leftmost = __bitree__::_S_minimum(_x);
            }
        }
        if (_rightmost == _s) {
            if (_s->_left == nullptr) {
                _rightmost = _s->_parent;
            }
            else {
                _rightmost = __bitree__::_S_maximum(_x);
            }
        }
    }

/**
 * @details
 *   %_y now point to the node to delete, which has been separated out.
 *   no matter with %_y and %_s in rebalance.
 *   %_x was the child of %_y
 *   if (_x) _x->_parent == _x_parent;
*/

    // rebalance
    if (_y->_color != _S_red) {
        // because %_y->_color == _S_black, so the sibling node of %_x can't be nullptr
        while (_x != _root && __rb_tree__::_S_as_black_node(_x)) {
            if (_x == _x_parent->_left) {
                node_type* _w = _x_parent->_right; // the sibling node of _x
                if (_w->_color == _S_red) { // case 4.1
                    _w->_color = _S_black;
                    _x_parent->_color = _S_red;
                    __bitree__::_S_left_rotate(_x_parent, &_m_impl._header);
                    _w = _x_parent->_right; // new sibling node of %_x
                }
                // %_w->_color == _S_black
                if (__rb_tree__::_S_as_black_node(_w->_left) && __rb_tree__::_S_as_black_node(_w->_right)) { // case 4.2
                    _w->_color = _S_red;
                    _x = _x_parent;
                    _x_parent = _x_parent->_parent;
                }
                else {
                    if (__rb_tree__::_S_as_black_node(_w->_right)) {
                        _w->_left->_color = _S_black;
                        _w->_color = _S_red;
                        __bitree__::_S_right_rotate(_w, &_m_impl._header);
                        _w = _x_parent->_right;
                    }
                    _w->_color = _x_parent->_color;
                    _x_parent->_color = _S_black;
                    if (_w->_right != nullptr) {
                        _w->_right->_color = _S_black;
                    }
                    __bitree__::_S_left_rotate(_x_parent, &_m_impl._header);
                    break;
                }
            }
            else { // same as above
                node_type* _w = _x_parent->_left;
                if (_w->_color == _S_red) {
                    _w->_color = _S_black;
                    _x_parent->_color = _S_red;
                    __bitree__::_S_right_rotate(_x_parent, &_m_impl._header);
                    _w = _x_parent->_left;
                }
                if (__rb_tree__::_S_as_black_node(_w->_right) && __rb_tree__::_S_as_black_node(_w->_left)) {
                    _w->_color = _S_red;
                    _x = _x_parent;
                    _x_parent = _x_parent->_parent;
                }
                else {
                    if (__rb_tree__::_S_as_black_node(_w->_left)) {
                        _w->_right->_color = _S_black;
                        _w->_color = _S_red;
                        __bitree__::_S_left_rotate(_w, &_m_impl._header);
                        _w = _x_parent->_left;
                    }
                    _w->_color = _x_parent->_color;
                    _x_parent->_color = _S_black;
                    if (_w->_left != nullptr) {
                        _w->_left->_color = _S_black;
                    }
                    __bitree__::_S_right_rotate(_x_parent, &_m_impl._header);
                    break;
                }
            }
        }
        if (_x != nullptr) _x->_color = _S_black;
    }

    return _y;
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
::_M_insert_unique_position(const key_type& _k) -> std::pair<node_type*, node_type*> {
    typedef std::pair<node_type*, node_type*> _Res;
    node_type* _x = _M_begin();
    node_type* _y = _M_end();
    bool _comp_res;
    while (_x != nullptr) {
        _y = _x;
        _comp_res = _M_key_compare(_k, _S_key(_x));
        _x = _comp_res ? _x->_left : _x->_right;
    }
    // _S_key(_y) is the closest to %_k
    // the new node should be inserted to the left or right side of node(_y)
    // depending on %_comp_res
    // if %_comp_res, %_k < _S_key(_y); or %_k >= _S_key(_y)
    iterator _j = iterator(_y);
    if (_comp_res) {
        if (_j == begin()) { // %_k not found
            return _Res(_x, _y);
        }
        --_j; // point to the previous node of %_y (notice that %_k < _S_key(_y))
    }
    // if %_k not exist in tree, _S_key(*_j) must be less (not equal) than %_k
    if (_M_key_compare(_S_key(_j._ptr), _k)) {
        return _Res(_x, _y);
    }
    return _Res(_j._ptr, nullptr);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert_multi_position(const key_type& _k) -> node_type* {
    node_type* _x = _M_begin();
    node_type* _y = _M_end();
    bool _comp_res;
    while (_x != nullptr) {
        _y = _x;
        _comp_res = _M_key_compare(_k, _S_key(_x));
        _x = _comp_res ? _x->_left : _x->_right;
    }
    return _y;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert(const value_type& _v, asp::true_type) -> std::pair<iterator, bool> {
    std::pair<node_type*, node_type*> _res = _M_insert_unique_position(_S_key(_v));
    if (_res.second != nullptr) {
        node_type* _x = this->_M_allocate_node(_v);
        _M_insert_rebalance(_res.second, _x);
        ++_m_impl._node_count;
        return std::make_pair(iterator(_x), true);
    }
    return std::make_pair(iterator(_res.first), false);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_insert(const value_type& _v, asp::false_type) -> iterator {
    node_type* _res = _M_insert_multi_position(_S_key(_v));
    node_type* _x = this->_M_allocate_node(_v);
    _M_insert_rebalance(_res, _x);
    ++_m_impl._node_count;
    return iterator(_x);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_erase(const_iterator _p) -> size_type {
    node_type* _s = _M_erase_rebalance(const_cast<node_type*>(_p._ptr));
    this->_M_deallocate_node(_s);
    --_m_impl._node_count;
    return 1;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_erase(const_iterator _first, const_iterator _last) -> size_type {
    size_type _ret = 0;
    if (_first == cbegin() && _last == cend()) {
        _ret = size();
        clear();
    }
    else {
        while (_first != _last) {
            _M_erase(_first++);
            ++_ret;
        }
    }
    return _ret;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_erase_subtree(node_type* _s) -> void {
    while (_s != nullptr) {
        _M_erase_subtree(_s->_right);
        node_type* _p = _s->_left;
        this->_M_deallocate_node(_s);
        _s = _p;
    }
};

/// rb_tree public implement
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::rb_tree(const self& _rbt) {
    _M_assign(_rbt, [this](const node_type* _n) -> node_type* {
        node_type* _p = this->_M_allocate_node(*_n);
        _p->_parent = nullptr; _p->_left = nullptr; _p->_right = nullptr;
        _p->_color = _n->_color;
        return _p;
    });
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::~rb_tree() {

};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
template <typename _NodeGen> void rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_assign(const self& _rbt, const _NodeGen& _gen) {
    _m_impl.reset();
    if (_rbt._M_begin() == nullptr) { return; }
    _m_impl._node_count = _rbt.size();
    node_type* _root = _M_clone_tree(_rbt._M_begin(), _M_end(), _gen);
    _m_impl._header._parent = _root;
    _m_impl._header._left = __bitree__::_S_minimum(_root);
    _m_impl._header._right = __bitree__::_S_maximum(_root);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc>
template <typename _NodeGen> auto rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>
::_M_clone_tree(const node_type* _x, node_type* _p, const _NodeGen& _gen) -> node_type* {
    node_type* _top = _gen(_x);
    _top->_parent = _p;

    if (_x->_left != nullptr) {
        _top->_left = _M_clone_tree(_x->_left, _top, _gen);
    }
    if (_x->_right != nullptr) {
        _top->_right = _M_clone_tree(_x->_right, _top, _gen);
    }

    return _top;
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::find(const key_type& _k)
-> iterator {
    iterator _j = _M_lower_bound(_M_begin(), _M_end(), _k);
    return (_j == end() || _M_key_compare(_k, _S_key(_j._ptr))) ? end() : _j;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::find(const key_type& _k) const
-> const_iterator {
    const_iterator _j = _M_lower_bound(_M_begin(), _M_end(), _k);
    return (_j == cend() || _M_key_compare(_k, _S_key(_j._ptr))) ? cend() : _j;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::count(const key_type& _k) const
-> size_type {
    std::pair<const_iterator, const_iterator> _res = equal_range(_k);
    const size_type _n = asp::distance(_res.first, _res.second);
    return _n;
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::clear()
-> void {
    _M_erase_subtree(_M_begin());
    _m_impl.reset();
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::insert(const value_type& _v)
-> ireturn_type {
    return this->_M_insert(_v, asp::bool_t<_UniqueKey>());
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::erase(const key_type& _k)
-> size_type {
    std::pair<const_iterator, const_iterator> _p = equal_range(_k);
    return this->_M_erase(_p.first, _p.second);
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::equal_range(const key_type& _k)
-> std::pair<iterator, iterator> {
    node_type* _x = _M_begin();
    node_type* _y = _M_end();
    while (_x != nullptr) {
        if (_M_key_compare(_S_key(_x), _k)) {
            _x = _x->_right;
        }
        else if (_M_key_compare(_k, _S_key(_x))) {
            _y = _x, _x = _x->_left;
        }
        else { // _S_key(_x) == _k
            node_type* _xu = _x;
            node_type* _yu = _y;
            _y = _x, _x = _x->_left;
            _xu = _xu->_right;
            return std::make_pair(
                iterator(_M_lower_bound(_x, _y, _k)),
                iterator(_M_upper_bound(_xu, _yu, _k)));
        }
    }
    return std::make_pair(iterator(_y), iterator(_y));
};
template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::equal_range(const key_type& _k) const
-> std::pair<const_iterator, const_iterator> {
    const node_type* _x = _M_begin();
    const node_type* _y = _M_end();
    while (_x != nullptr) {
        if (_M_key_compare(_S_key(_x), _k)) {
            _x = _x->_right;
        }
        else if (_M_key_compare(_k, _S_key(_x))) {
            _y = _x, _x = _x->_left;
        }
        else { // _S_key(_x) == _k
            const node_type* _xu = _x;
            const node_type* _yu = _y;
            _y = _x, _x = _x->_left;
            _xu = _xu->_right;
            return std::make_pair(
                const_iterator(_M_lower_bound(_x, _y, _k)),
                const_iterator(_M_upper_bound(_xu, _yu, _k)));
        }
    }
    return std::make_pair(const_iterator(_y), const_iterator(_y));
};

template <typename _Key, typename _Value, typename _ExtKey, bool _UniqueKey, typename _Comp, typename _Alloc> auto
rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc>::check() const -> int {
    typedef rb_tree<_Key, _Value, _ExtKey, _UniqueKey, _Comp, _Alloc> rb_tree_t;
    auto _bt_check = __bitree__::_S_check<_Comp, typename rb_tree_t::ext_key>(&_m_impl._header, _m_impl._node_count);
    auto _rb_check = __rb_tree__::_S_check(&_m_impl._header);
    return _bt_check + (_rb_check>0 ? 100 : 0) + _rb_check;
};


/// output implement
template <typename _K, typename _V, typename _EK, bool _UK, typename _C, typename _A>
std::ostream& operator<<(std::ostream& os, const rb_tree<_K, _V, _EK, _UK, _C, _A>& _r) {
    os << '[';
    for (auto p = _r.cbegin(); p != _r.cend();) {
        os << p;
        if (++p != _r.cend()) {
            os << ", ";
        }
    }
    os << ']';
    return os;
};
template <typename _T> std::ostream& operator<<(std::ostream& os, const rb_tree_iterator<_T>& _r) {
    if (_r)
        os << obj_string::_M_obj_2_string(*_r);
    else 
        os << "null";
    return os;
};
template <typename _T> std::ostream& operator<<(std::ostream& os, const rb_tree_const_iterator<_T>& _r) {
    if (_r)
        os << obj_string::_M_obj_2_string(*_r);
    else
        os << "null";
    return os;
};


/// __rb_tree__ implement
namespace __rb_tree__ {
template <typename _Tp> bool _S_as_black_node(const rb_tree_node<_Tp>* _x) {
    return _x == nullptr || _x->_color == _S_black;
};

/**
 * @brief check the rb_tree's 5 rules
 * @returns 0 : normal ;
 *   1 : root node isn't black ;
 *   2 : error in black height (break 5th rule) ;
 *   3 : red node has at least one red child (break 4th rule) .
*/
template <typename _Tp> int _S_check(const rb_tree_node<_Tp>* _header) {
    typedef rb_tree_node<_Tp> node_type;
    const node_type* _root = _header->_parent;
    if (_root == nullptr) { return 0; }
    if (_root->_color != _S_black) {
        return 1;
    }
    int _bh = _S_black_height(_root);
    if (_bh == -1) { return 2; }
    else if (_bh == -2) { return 3; }
    return 0;
};

template <typename _Tp> int _S_black_height(const rb_tree_node<_Tp>* _s, int _bh) {
    if (_s == nullptr) { return _bh; }
    if (_s->_color == _S_red) {
        if (!_S_as_black_node(_s->_left) || !_S_as_black_node(_s->_right)) {
            return -2;
        }
    }
    else {
        ++_bh;
    }
    int _l = _S_black_height(_s->_left, _bh);
    if (_l < 0) { return _l; }
    int _r = _S_black_height(_s->_right, _bh);
    if (_r < 0) { return _r; }
    if (_l != _r) { return -1; }
    return _l;
}
};

};

#endif