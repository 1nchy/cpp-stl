#ifndef _ASP_ITERATOR_HPP_
#define _ASP_ITERATOR_HPP_

#include <iterator>

#include "basic_param.hpp"
#include "iterator_traits.hpp"

namespace asp {
template <typename _Tp, typename _Pointer, typename _Reference> struct base_iterator;
template <typename _Tp, typename _Pointer = _Tp*, typename _Reference = _Tp&> struct base_iterator {
    // typedef typename _Tp value_type;
    // typedef typename _Pointer pointer;
    // typedef typename _Reference reference;
    typedef asp::fixed_iterator iterator_category;
    using value_type = _Tp;
    using pointer = _Pointer;
    using reference = _Reference;
    using difference_type = asp::difference_type;
    typedef base_iterator<_Tp> self;

    base_iterator() = default;
    base_iterator(const pointer _p) : _ptr(_p) {}
    base_iterator(const self& _s) : _ptr(_s._ptr) {}
    self& operator=(const self& _s) {
        this->_ptr = _s._ptr;
    }

    reference operator*() {
        return *_ptr;
    }
    pointer operator->() {
        return _ptr;
    }
    const pointer& base() const {
        return _ptr;
    }

    friend bool operator==(const self& lhs, const self& rhs) {
        return lhs._ptr == rhs._ptr;
    }
    friend bool operator!=(const self& lhs, const self& rhs) {
        return lhs._ptr != rhs._ptr;
    }

    value_type* _ptr = nullptr;
};

template <typename _Tp> 
struct normal_iterator;

template <typename _Tp> 
struct normal_iterator : public base_iterator <_Tp> {
    typedef asp::random_access_iterator_tag iterator_category;
    // typedef value_type = base_iterator<_Tp>::value_type;
    // typedef pointer = base_iterator<_Tp>::pointer;
    // typedef reference = base_iterator<_Tp>::reference;
    typedef typename base_iterator<_Tp>::value_type value_type;
    typedef typename base_iterator<_Tp>::pointer pointer;
    typedef typename base_iterator<_Tp>::reference reference;
    typedef base_iterator<_Tp> base;
    typedef normal_iterator<value_type> self;
    typedef typename base::difference_type difference_type;

    using base_iterator<_Tp>::_ptr;

    normal_iterator() = default;
    normal_iterator(const pointer _p): base(_p) {}
    normal_iterator(const self& rhs): base(rhs._ptr) {}
    self& operator=(const self& rhs) {
        _ptr = rhs._ptr;
    }
    // @iterator to @const_iterator conversion
    template <typename _Iter> normal_iterator(const normal_iterator<_Iter>& _i)
     : base(_i._ptr) {}
    
    self& operator++() {
        ++_ptr;
        return *this;
    }
    self operator++(int) {
        self _tmp = *this;
        ++_ptr;
        return _tmp;
    }
    self& operator--() {
        --_ptr;
        return *this;
    }
    self operator--(int) {
        self _tmp = *this;
        --_ptr;
        return _tmp;
    }
    self operator+(int _n) {
        self _tmp = *this;
        _tmp._ptr += _n;
        return _tmp;
    }
    self operator-(int _n) {
        self _tmp = *this;
        _tmp._ptr -= _n;
        return _tmp;
    }
    friend difference_type operator-(const self& last, const self& first) {
        return last._ptr - first._ptr;
    }
// private:
    // value_type* _ptr = nullptr;
};

template <typename _Iterator> static difference_type _distance(_Iterator& first, _Iterator& last, asp::random_access_iterator_tag) {
    return last - first;
}
template <typename _Iterator> static difference_type _distance(_Iterator& first, _Iterator& last, asp::bidirectional_iterator_tag) {
    difference_type _n = 0;
    while (first != last) {
        ++first;
        ++_n;
    }
    return _n;
}
template <typename _Iterator, typename _UnknownIteraterTag> static difference_type _distance(_Iterator& first, _Iterator& last, _UnknownIteraterTag) {
    return std::distance(first, last);
}

template <typename _Tp> static void _advance(normal_iterator<_Tp>& _p, difference_type _n, asp::random_access_iterator_tag) {
    _p += _n;
}
template <typename _Iterator> static void _advance(_Iterator& _p, difference_type _n, asp::bidirectional_iterator_tag) {
    if (_n > 0) {
        while (_n--) {
            ++_p;
        }
    }
    else if (_n < 0) {
        while (_n++) {
            --_p;
        }
    }
}
template <typename _Iterator, typename _UnknownIteraterTag> static void _advance(_Iterator& _p, difference_type _n, _UnknownIteraterTag) {
    std::advance(_p, _n);
}


template <typename _Iterator> difference_type distance(_Iterator first, _Iterator last) {
    return _distance(first, last, asp::_A_iterator_category(first));
}
template <typename _Iterator> void advance(_Iterator& _p, difference_type _n) {
    _advance(_p, _n, asp::_A_iterator_category(_p));
}

};

#endif