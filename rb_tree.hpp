#ifndef _RB_TREE_HPP_
#define _RB_TREE_HPP_

#include "tree_node.hpp"

#include "memory.hpp"
// #include <memory>

namespace asp {

enum _Rb_tree_color { _S_red = false, _S_black = true };
template <typename _Tp> struct rb_tree_node;
template <typename _Tp> struct rb_tree_header;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct rb_tree_base;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct rb_tree;

template <typename _Tp> struct rb_tree_node : public bitree_node<_Tp> {
    typedef bitree_node<_Tp> base;
    typedef rb_tree_node<_Tp> self;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using reference = typename base::reference;
    _Rb_tree_color _color;

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
template <typename _Tp> struct rb_tree_header {
    rb_tree_node<_Tp> _header;
    size_type _node_count;

    rb_tree_header() {
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
        _node_count = _from._node_count;
        _from.reset();
    }

    void reset() {
        _header._parent = nullptr;
        _header._left = &_header;
        _header._right = &_header;
        _node_count = 0;
    }
};

template <typename _Tp, typename _Alloc> struct rb_tree_base {};
template <typename _Tp, typename _Alloc> struct rb_tree : public rb_tree_base<_Tp, _Alloc> {};

};

#endif