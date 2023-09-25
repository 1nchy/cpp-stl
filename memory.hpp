#ifndef _ASP_MEMORY_HPP_
#define _ASP_MEMORY_HPP_

#include <memory>
#include "iterator_traits.hpp"

namespace asp {

template <typename _InputIterator, typename _ForwardIterator, typename _Allocator>
_ForwardIterator _A_copy(_InputIterator _first, _InputIterator _last, _ForwardIterator _res, _Allocator& _a) {
    _ForwardIterator _cur = _res;
    typedef std::allocator_traits<_Allocator> _alloc_traits;
    for (; _first != _last; ++_first, ++_cur) {
        _alloc_traits::construct(_a, std::addressof(*_cur), *_first);
    }
    return _cur;
}

template <typename _InputIterator, typename _Tp>
_Tp* _A_copy(_InputIterator _first, _InputIterator _last, _Tp* _res) {
    _Tp* _cur = _res;
    for (; _first != _last; ++_first, ++_cur) {
        *(std::addressof(*_cur)) = *_first;
    }
    return _cur;
}

template <typename _InputIterator, typename _BackwardIterator, typename _Allocator>
_BackwardIterator _A_copy_backward(_InputIterator _first, _InputIterator _last, _BackwardIterator _res, _Allocator& _a) {
    _BackwardIterator _cur = _res;
    typedef std::allocator_traits<_Allocator> _alloc_traits;
    while (_first != _last) {
        --_cur; --_last;
        _alloc_traits::construct(_a, std::addressof(*_cur), *_last);
    }
    return _cur;
}

template <typename _InputIterator, typename _Tp>
_Tp _A_copy_backward(_InputIterator _first, _InputIterator _last, _Tp* _res) {
    _Tp* _cur = _res;
    while (_first != _last) {
        *(std::addressof(*--_cur)) = *--_last;
    }
    return _cur;
}

};

#endif