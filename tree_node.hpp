#ifndef _ASP_TREE_NODE_HPP_
#define _ASP_TREE_NODE_HPP_

#include "node.hpp"

namespace asp {

template <typename _Tp> struct bitree_node;

template <typename _Tp> struct bitree_node : node<_Tp> {
    typedef node<_Tp> base;
    typedef bitree_node<_Tp> self;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using reference = typename base::reference;

    bitree_node() : base() {}
    bitree_node(const self& rhs) : base(rhs),
     _parent(rhs._parent), _left(rhs._left), _right(rhs._right) {}
    bitree_node(self&& rhs) : base(std::move(rhs)),
     _parent(rhs._parent), _left(rhs._left), _right(rhs._right) {}
    self& operator=(const self& rhs) {
        base::operator=(rhs);
        _parent = rhs._parent;
        _left = rhs._left;
        _right = rhs._right;
        return *this;
    }
    self& operator=(self&& rhs) {
        base::operator=(std::move(rhs));
        _parent = rhs._parent;
        _left = rhs._left;
        _right = rhs._right;
        return *this;
    }
    virtual ~bitree_node() {
        reset();
    }

    void reset() { _parent = nullptr; _left = nullptr; _right = nullptr; }

    void hook_left(self* const _p) {
        
    }
    void hook_right(self* const _p) {

    }
    void hook_parent(self* const _p) {

    }

    self* _parent = nullptr;
    self* _left = nullptr;
    self* _right = nullptr;
};

};

#endif