#ifndef _ASP_TREE_NODE_HPP_
#define _ASP_TREE_NODE_HPP_

#include "node.hpp"

namespace asp {

template <typename _Tp> struct bitree_node;
template <typename _Tp> struct bitree_header;

/**
 * @brief the canonical structure of an ordered binary tree
 * @details
 *    l┌──────  _header  ───────┐r
 *     │        p ↓↑ p          |
 *     │         _root          │
 *     │    l ↓↑ p,    r ↓↑ p   │
 *     │    _left      _right   │
 *     ↓      ..........        ↓
 *   _min                     _max
 * 
 *   _header._left = _min (minium node)
 *   _header._right = _max (maxium node)
 *   _header._parent = _root
 *   _root._parent = _header;
 *   _min._left = _max._right = nullptr
*/

namespace __bitree__ {
// maximum node in subtree rooted at %_x, _Node must be derived class of bitree_node
template <typename _Node> _Node* _S_maximum(_Node* _x);
// minimum node in subtree rooted at %_x, _Node must be derived class of bitree_node
template <typename _Node> _Node* _S_minimum(_Node* _x);

template <typename _Node> void _S_left_rotate(_Node* _x, _Node* _header);
template <typename _Node> void _S_right_rotate(_Node* _x, _Node* _header);
/**
 * @brief find the least node (r) greater than %_x
 * @details 4 cases :
 *   1.   r  2.    r (_header)  3.  r (_header)  4. _x
 *    | p     |  _root           |  _x (_root)    |    r
 *    |  _x   |       _x         |                |
*/
template <typename _Node> _Node* _S_bitree_node_increase(_Node* _x);
/**
 * @brief find the greatest node (r) less than %_x
 * @details 4 cases :
 *   1.   _x  2.   _x (_header) 3.  r    4.   r (_header)
 *    | p      | _root           |    p   |   p (_root)
 *    |   r    |  ...   r        | _x     | _x
*/
template <typename _Node> _Node* _S_bitree_node_decrease(_Node* _x);

/**
 * @brief check the order of binary tree.
*/
template <typename _Node> int _S_check(const _Node* _header, size_type _n);
};

template <typename _Tp> struct bitree_node : node<_Tp> {
    typedef node<_Tp> base;
    typedef bitree_node<_Tp> self;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using reference = typename base::reference;

    self* _parent = nullptr;
    self* _left = nullptr;
    self* _right = nullptr;

    bitree_node() : base() {}
    bitree_node(const self& rhs) : base(rhs),
     _parent(rhs._parent), _left(rhs._left), _right(rhs._right) {}
    bitree_node(self&& rhs) : base(std::move(rhs)),
     _parent(rhs._parent), _left(rhs._left), _right(rhs._right) {}
    bitree_node(const value_type& _v) : base(_v) {}
    template <typename... _Args> bitree_node(_Args&&... _args) : base(std::forward<_Args>(_args)...) {}
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
};


template <typename _Tp> struct bitree_header {
    typedef bitree_node<_Tp> _Node;
    typedef typename _Node::value_type value_type;
    bitree_node<_Tp> _header;
    size_type _node_count;

    bitree_header() {
        reset();
    }

    void reset() {
        this->_header._parent = nullptr;
        this->_header._left = &this->_header;
        this->_header._right = &this->_header;
        this->_node_count = 0;
    }
};



///  __bitree__ implement
namespace __bitree__ {
template <typename _Node> _Node* _S_maximum(_Node* _x) {
    while (_x->_right != nullptr) {
        _x = _x->_right;
    }
    return _x;
};
template <typename _Node> _Node* _S_minimum(_Node* _x) {
    while (_x->_left != nullptr) {
        _x = _x->_left;
    }
    return _x;
};

template <typename _Node> void _S_left_rotate(_Node* _x, _Node* _header) {
    _Node* _right_child = _x->_right;
    if (_right_child == nullptr) {
        return ;
    }
    _Node* _this_parent = _x->_parent;
    _right_child->_parent = _this_parent;
    if (_this_parent != nullptr) {
        if (_x == _header->_parent) {
            _header->_parent = _right_child;
        }
        else if (_x == _this_parent->_left) {
            _this_parent->_left = _right_child;
        }
        else {
            _this_parent->_right = _right_child;
        }
    }
    _x->_right = _right_child->_left;
    if (_right_child->_left != nullptr) {
        _right_child->_left->_parent = _x;
    }
    _right_child->_left = _x;
    _x->_parent = _right_child;
}
template <typename _Node> void _S_right_rotate(_Node* _x, _Node* _header) {
    _Node* _left_child = _x->_left;
    if (_left_child == nullptr) {
        return ;
    }
    _Node* _this_parent = _x->_parent;
    _left_child->_parent = _this_parent;
    if (_this_parent != nullptr) {
        if (_x == _header->_parent) {
            _header->_parent = _left_child;
        }
        else if (_x == _this_parent->_left) {
            _this_parent->_left = _left_child;
        }
        else {
            _this_parent->_right = _left_child;
        }
    }
    _x->_left = _left_child->_right;
    if (_left_child->_right != nullptr) {
        _left_child->_right->_parent = _x;
    }
    _left_child->_right = _x;
    _x->_parent = _left_child;
}

template <typename _Node> _Node* _S_bitree_node_increase(_Node* _x) {
    if (_x->_right != nullptr) {  // for case 4
        _x = _x->_right;
        while (_x->_left != nullptr) {
            _x = _x->_left;
        }
    }
    else {
        _Node* _y = _x->_parent;
        while (_y != nullptr && _x == _y->_right) {
            // for case 1, 2
            // find the first node in %_x's ancestor nodes,
            // whose r-child's power won't equal to the original %_x
            _x = _y;
            _y = _y->_parent;
        }
        if (_x->_right != _y) { // for case 3, used for bitree_header, which manage the nodes
            _x = _y;
        }
    }
    return _x;
};
template <typename _Node> _Node* _S_bitree_node_decrease(_Node* _x) {
    if (_x->_parent->_parent == _x) {  // for case 2
        _x = _x->_right;
    }
    else if (_x->_left != nullptr) {  // for case 1
        _Node* _y = _x->_left;
        while (_y->_right != nullptr) {
            _y = _y->_right;
        }
        _x = _y;
    }
    else {
        _Node* _y = _x->_parent;
        while (_y != nullptr && _x == _y->_left) {  // for case 3, 4
            // find the first node in %_x's ancestor nodes,
            // whose l-child's power won't equal to the original %_x
            _x = _y;
            _y = _y->_parent;
        }
        _x = _y;
    }
    return _x;
};

/**
 * @brief check the order of binary tree.
 * @returns 0 : normal ;
 *   1 : error in left/right-most ;
 *   2 : error in order ;
 *   3 : error in %_node_count .
*/
template <typename _Node> int _S_check(const _Node* _header, size_type _n) {
    const _Node* _root = _header->_parent;
    if (_root == nullptr) { return 0; }
    const _Node* _leftmost = _S_minimum(_root);
    const _Node* _rightmost = _S_maximum(_root);
    if (_header->_left != _leftmost || _header->_right != _rightmost) {
        return 1;
    }
    const _Node* _p = _root;
    std::vector<const _Node*> _vn;
    std::vector<const _Node*> _traverse_node;
    while (_p != nullptr || !_vn.empty()) {
        if (_p != nullptr) {
            _vn.push_back(_p);
            _p = _p->_left;
        }
        else {
            _p = _vn.back();
            _vn.pop_back();
            _traverse_node.push_back(_p);
            _p = _p->_right;
        }
    }
    if (_traverse_node.size() != _n) { return 3; }
    for (int _i = 0; _i < _traverse_node.size() - 1; ++_i) {
        if (_traverse_node[_i]->val() > _traverse_node[_i+1]->val()) {
            return 2;
        }
    }
    return 0;
};
};
};

#endif