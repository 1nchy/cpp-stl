#ifndef _ASP_SKIP_LIST_NODE_HPP_
#define _ASP_SKIP_LIST_NODE_HPP_

#include "basic_param.hpp"
#include "node.hpp"

namespace asp {

template <typename _Value> struct skip_list_node;

template <typename _Value> struct skip_list_node : public node<_Value> {
    typedef node<_Value> base;
    typedef skip_list_node<_Value> self;
    typedef typename base::value_type value_type;
    typedef typename base::pointer pointer;
    typedef typename base::reference reference;

    skip_list_node() : base() {}
    skip_list_node(const self& _n) : base(_n) {}
    skip_list_node(self&& _n) : base(std::move(_n)) {}
    skip_list_node(const value_type& _v) : base(_v) {}
    template <typename... _Args> skip_list_node(_Args&&... _args) : base(std::forward<_Args>(_args)...) {}
    virtual ~skip_list_node() {}

    self* _next[1] = { nullptr };
    self* _prev = nullptr;
    size_type _height = 1;

    void reset() {
        _prev = nullptr;
        _next[0] = nullptr;
    }
};


};

#endif  // _ASP_SKIP_LIST_NODE_HPP_