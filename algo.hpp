#ifndef _ASP_ALGO_HPP_
#define _ASP_ALGO_HPP_

#include "basic_param.hpp"
#include "iterator.hpp"
#include "basic_io.hpp"
#include "random.hpp"

#include <iostream>
#include <vector>

namespace asp {
template <typename _RandomAccessIterator, typename _Compare> typename asp::iterator_traits<_RandomAccessIterator>::value_type kth(_RandomAccessIterator _begin, _RandomAccessIterator _end, size_type _k, _Compare _comp);
template <typename _RandomAccessIterator> typename asp::iterator_traits<_RandomAccessIterator>::value_type kth(_RandomAccessIterator _begin, _RandomAccessIterator _end, size_type _k);
template <typename _RandomAccessIterator, typename _Compare> void sort(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp);
template <typename _RandomAccessIterator> void sort(_RandomAccessIterator _begin, _RandomAccessIterator _end);
template <typename _RandomAccessIterator> void shuffle(_RandomAccessIterator _begin, _RandomAccessIterator _end);

namespace __details__ {
static const size_type _S_quick_sort_threshold = size_type(16);
static const size_type _S_select_threshold = size_type(5);
// sort [_begin, _end) so that _comp(*p, *q) == true
template <typename _RandomAccessIterator, typename _Compare> void _A_quick_sort(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp);
template <typename _RandomAccessIterator, typename _Compare> void _A_insertion_sort(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp);

template <typename _RandomAccessIterator, typename _Compare> _RandomAccessIterator _A_partition(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp);
// partition [_begin, _end) with _pivot
template <typename _RandomAccessIterator, typename _Compare> _RandomAccessIterator _A_pivot_partition(_RandomAccessIterator _pivot, _RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp);
// *_last \in [_x, _last)\uarr, insert _last into [_x, _last) with linear order
template <typename _RandomAccessIterator, typename _Compare> _RandomAccessIterator _A_linear_insert(_RandomAccessIterator _last, _Compare _comp);
template <typename _RandomAccessIterator, typename _Compare> _RandomAccessIterator _A_partition_kth(_RandomAccessIterator _begin, _RandomAccessIterator _end, size_type _k, _Compare _comp);

template <typename _RandomAccessIterator, typename _Compare> void
_A_quick_sort(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp) {
    while (_end - _begin > 1) {
        _RandomAccessIterator _pivot = _A_partition(_begin, _end, _comp);
        _A_quick_sort(_pivot + 1, _end, _comp);
        _end = _pivot;
    }
};
template <typename _RandomAccessIterator, typename _Compare> void
_A_insertion_sort(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp) {
    if (_begin == _end) return;
    for (_RandomAccessIterator _i = _begin + 1; _i != _end; ++_i) {
        if (_comp(*_i, *_begin)) { // *_begin >= *_i
            typename asp::iterator_traits<_RandomAccessIterator>::value_type
             _val = std::move(*_i);
            std::move_backward(_begin, _i, _i + 1);
            *_begin = std::move(_val);
        }
        else {
            _A_linear_insert(_i, _comp);
        }
    }
};
template <typename _RandomAccessIterator, typename _Compare> auto
_A_partition(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp)->_RandomAccessIterator {
    if (_end - _begin <= 1) return _begin;
    _RandomAccessIterator _pivot = _begin;
    return _A_pivot_partition(_pivot, _begin + 1, _end, _comp);
};
template <typename _RandomAccessIterator, typename _Compare> auto
_A_pivot_partition(_RandomAccessIterator _pivot, _RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp)->_RandomAccessIterator {
    while (true) {
        while (_end - _begin > 0 && _comp(*_begin, *_pivot)) ++_begin; // ++_begin until *_begin >= *_pivot
        --_end;
        while (_end - _begin > 0 && _comp(*_pivot, *_end)) --_end; // --_end until *_pivot >= *_end
        // _pivot >= _end
        if (_end - _begin <= 0) {
            --_begin;
            asp::iter_swap(_begin, _pivot);
            return _begin;
        }
        asp::iter_swap(_begin, _end);
        ++_begin;
    }
};

template <typename _RandomAccessIterator, typename _Compare> _RandomAccessIterator
_A_linear_insert(_RandomAccessIterator _last, _Compare _comp) {
    typename asp::iterator_traits<_RandomAccessIterator>::value_type
     _val = std::move(*_last);
    _RandomAccessIterator _next = _last; --_next;
    while (_comp(_val, *_next)) { // _val < *_next
        *_last = std::move(*_next);
        _last = _next;
        --_next;
    }
    *_last = std::move(_val);
    return _last;
};


template <typename _RandomAccessIterator, typename _Compare> _RandomAccessIterator _A_partition_kth(_RandomAccessIterator _begin, _RandomAccessIterator _end, size_type _k, _Compare _comp) {
    while (true) {
        if (_end - _begin < int(_S_select_threshold)) {
            __details__::_A_insertion_sort(_begin, _end, _comp);
            return _begin + _k;
        }
        const size_type _n = (_end - _begin) / (_S_select_threshold); // the number of group
        for (size_type _i = 0; _i < _n; ++_i) {
            __details__::_A_insertion_sort(_begin + _i * _S_select_threshold, _begin + (_i + 1) * _S_select_threshold, _comp);
            asp::iter_swap(_begin + _i, _begin + _i * _S_select_threshold + _S_select_threshold / 2);
        }
        _RandomAccessIterator _pivot = _A_partition_kth(_begin, _begin + _n, _n / 2, _comp);
        asp::iter_swap(_begin, _pivot);
        _pivot = _A_pivot_partition(_begin, _begin + 1, _end, _comp);
        if (_pivot - _begin == _k) {
            return _pivot;
        }
        else if (_pivot - _begin > _k) {
            _end = _pivot;
        }
        else { // _pivot - _begin < _k
            _k -= _pivot - _begin;
            _begin = _pivot;
        }
    }
};
};


template <typename _RandomAccessIterator, typename _Compare> auto
kth(_RandomAccessIterator _begin, _RandomAccessIterator _end, size_type _k, _Compare _comp)
-> typename asp::iterator_traits<_RandomAccessIterator>::value_type {
    return *(__details__::_A_partition_kth(_begin, _end, _k, _comp));
};
template <typename _RandomAccessIterator> auto
kth(_RandomAccessIterator _begin, _RandomAccessIterator _end, size_type _k)
-> typename asp::iterator_traits<_RandomAccessIterator>::value_type {
    return *(__details__::_A_partition_kth(_begin, _end, _k, std::less<typename asp::iterator_traits<_RandomAccessIterator>::value_type>()));
};
template <typename _RandomAccessIterator, typename _Compare> void sort(_RandomAccessIterator _begin, _RandomAccessIterator _end, _Compare _comp) {
    __details__::_A_quick_sort(_begin, _end, _comp);
    // __details__::_A_insertion_sort(_begin, _end, _comp);
};
template <typename _RandomAccessIterator> void sort(_RandomAccessIterator _begin, _RandomAccessIterator _end) {
    __details__::_A_quick_sort(_begin, _end, std::less<typename asp::iterator_traits<_RandomAccessIterator>::value_type>());
    // __details__::_A_insertion_sort(_begin, _end, std::less<typename asp::iterator_traits<_RandomAccessIterator>::value_type>());
};
template <typename _RandomAccessIterator> void shuffle(_RandomAccessIterator _begin, _RandomAccessIterator _end) {
    if (_end - _begin <= 0) return;
    _RandomAccessIterator _last = _end - 1;
    for (size_type _k = _end - _begin; _k > 0; --_k) {
        _RandomAccessIterator _choosed = _begin + asp::_S_random_unsigned(_k);
        asp::iter_swap(_choosed, _last);
        --_last;
    }
};


};





#endif // _ASP_ALGO_HPP_