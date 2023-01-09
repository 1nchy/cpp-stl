#ifndef _ASP_SKIP_LIST_NODE_HPP_
#define _ASP_SKIP_LIST_NODE_HPP_

#include "basic_param.hpp"
#include "node.hpp"

namespace asp {

template <typename _Tp> struct skip_list_node;

template <typename _Tp> struct skip_list_node : public node<_Tp> {
    typedef node<_Tp> base;
    typedef skip_list_node<_Tp> self;
    typedef typename base::value_type value_type;
    typedef typename base::pointer pointer;
    typedef typename base::reference reference;
};


};

#endif  // _ASP_SKIP_LIST_NODE_HPP_