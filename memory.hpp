#ifndef _ASP_MEMORY_HPP_
#define _ASP_MEMORY_HPP_

#include <memory>

namespace asp {

template<typename _InputIterator, typename _ForwardIterator, typename _Allocator>
_ForwardIterator _A_uninitialized_copy_a(_InputIterator _first, _InputIterator _last, _ForwardIterator _res, _Allocator& _a) {
    _ForwardIterator _cur = _res;
    typedef std::allocator_traits<_Allocator> _alloc_traits;
    for (; _first != _last; ++_first, ++_cur) {
        _alloc_traits::construct(_a, std::addressof(*_cur), *_first);
    }
    return _cur;
}

};

#endif