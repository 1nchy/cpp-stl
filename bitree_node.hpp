#ifndef _ASP_BITREE_NODE_HPP_
#define _ASP_BITREE_NODE_HPP_

#include <utility>

#include "node.hpp"

namespace asp {
template <typename _Tp> struct bitree_node;

template <typename _Tp> struct bitree_node : public node<_Tp> {
    // typedef typename node<_Tp>::value_type value_type;
    using value_type = typename node<_Tp>::value_type;
    using pointer = typename node<_Tp>::pointer;
    using reference = typename node<_Tp>::reference;
    typedef bitree_node<value_type> btnode;

    bitree_node(): node<value_type>() {}
    bitree_node(const value_type& n): node<value_type>(n) {}
    bitree_node(const btnode& rhs): node<value_type>(rhs),
        parent(rhs.parent), left(rhs.left), right(rhs.right) {}
    bitree_node(btnode&& rhs): node<value_type>(std::move(rhs)),
        parent(rhs.parent), left(rhs.left), right(rhs.right) {}
    btnode& operator=(const btnode& rhs) {
        node<value_type>::operator=(rhs);
        parent = rhs.parent;
        left = rhs.left;
        right = rhs.right;
        return *this;
    }
    btnode& operator=(btnode&& rhs) {
        node<value_type>::operator=(std::move(rhs));
        parent = rhs.parent;
        left = rhs.left;
        right = rhs.right;
        return *this;
    }
    virtual ~bitree_node() {
        reset();
    }
    void reset() { parent = nullptr; left = nullptr; right = nullptr; }


    btnode* parent = nullptr;
    btnode* left = nullptr;
    btnode* right = nullptr;
};

};

#endif //! #ifndef _ASP_BITREE_NODE_HPP_