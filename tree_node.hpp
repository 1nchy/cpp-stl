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

    using base::_pt;

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
    self* left_rotate() {
        self* _right_child = this->_right;
        if (_right_child == nullptr) {
            return this;
        }
        self* _this_parent = this->_parent;
        _right_child->_parent = _this_parent;
        if (_this_parent != nullptr) {
            if (this == _this_parent->_left) {
                _this_parent->_left = _right_child;
            }
            else {
                _this_parent->_right = _right_child;
            }
        }
        this->_right = _right_child->_left;
        if (_right_child->_left != nullptr) {
            _right_child->_left->_parent = this;
        }
        _right_child->_left = this;
        this->_parent = _right_child;
        return _right_child;
    }
    self* right_rotate() {
        self* _left_child = this->_left;
        if (_left_child == nullptr) {
            return this;
        }
        self* _this_parent = this->_parent;
        _left_child->_parent = _this_parent;
        if (_this_parent != nullptr) {
            if (this == _this_parent->_left) {
                _this_parent->_left = _left_child;
            }
            else {
                _this_parent->_right = _left_child;
            }
        }
        this->_left = _left_child->_right;
        if (_left_child->_right != nullptr) {
            _left_child->_right->_parent = this;
        }
        _left_child->_right = this;
        this->_parent = _left_child;
        return _left_child;
    }

protected:
    bool check() const {
        if (this->_left != nullptr && this->_left->_parent != this) {
            return false;
        }
        if (this->_right != nullptr && this->_right->_parent != this) {
            return false;
        }
        return (this->_left == nullptr || this->_left->check()) && (this->_right == nullptr || this->_right->check());
    }

    self* _parent = nullptr;
    self* _left = nullptr;
    self* _right = nullptr;
};

};

#endif