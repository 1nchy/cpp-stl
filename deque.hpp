#ifndef  _ASP_DEQUE_HPP_
#define _ASP_DEQUE_HPP_

#include <memory>

#include "deque_base.hpp"

namespace asp {

template <typename _Tp, typename _Alloc> class deque;

template <typename _Tp, typename _Alloc = std::allocator<_Tp>> class deque : public deque_base<_Tp, _Alloc> {

};

};

#endif