#ifndef  _ASP_DEQUE_HPP_
#define _ASP_DEQUE_HPP_

#include <memory>

#include "deque_base.hpp"

namespace asp {

template <typename _Tp, typename _Alloc = std::allocator<_Tp>> class deque;

template <typename _Tp, typename _Alloc> class deque : public deque_base<_Tp, _Alloc> {
    typedef _Tp value_type;
    typedef deque_base<_Tp, _Alloc> base;
    typedef deque<_Tp, _Alloc> self;
    typedef _Alloc allocator_type;

    typedef typename base::pointer pointer;
    typedef typename base::reference reference;
    typedef typename base::alloc_traits alloc_traits;

    using base::_data;
};

};

#endif