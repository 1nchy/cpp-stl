#ifndef _RB_TREE_HPP_
#define _RB_TREE_HPP_

#include "tree_node.hpp"

#include "memory.hpp"
// #include <memory>

namespace asp {

template <typename _Tp> struct rb_tree_node;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct rb_tree_base;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct rb_tree;

template <typename _Tp> struct rb_tree_node : public bitree_node<_Tp> {};
template <typename _Tp, typename _Alloc> struct rb_tree_base {};
template <typename _Tp, typename _Alloc> struct rb_tree : public rb_tree_base<_Tp, _Alloc> {};

};

#endif