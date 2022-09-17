#ifndef  _ASP_DEQUE_HPP_
#define _ASP_DEQUE_HPP_

#include <memory>

#include "deque_base.hpp"

namespace asp {

template <typename _Tp, typename _Alloc = std::allocator<_Tp>> class deque;

template <typename _Tp, typename _Alloc> class deque : public deque_base<_Tp, _Alloc> {
    typedef deque_base<_Tp, _Alloc> base;
    typedef deque<_Tp, _Alloc> self;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using reference = typename base::reference;
    using elt_pointer = typename base::elt_pointer;
    using map_pointer = typename base::map_pointer;
    using elt_allocator_type = typename base::elt_allocator_type;
    using map_allocator_type = typename base::map_allocator_type;

    using iterator = typename base::iterator;
    using const_iterator = typename base::const_iterator;

    using base::_data;

    deque() : base() {}
    explicit deque(const elt_allocator_type& _a) : base(_a) {}
    explicit deque(size_type _n, const elt_allocator_type& _a = elt_allocator_type())
     : base(_a, _n) {}
    deque(const self& _x) : base(_x._M_get_elt_allocator(), _x.size()) {}
    deque(self&& _x) : base(std::move(_x)) {}
    ~deque() = default;

    self& operator=(const deque& _x);
    self& operator=(deque&& _x);


    /// element access
    reference operator[](size_type _i) {
        return *(this->_data._start + difference_type(_i));
    }
    const reference operator[](size_type _i) const {
        return *(this->_data._start + difference_type(_i));
    }
    reference front() { return *begin(); }
    const reference front() const { return *cbegin(); }
    reference back() {
        iterator _tmp = end();
        --_tmp;
        return *_tmp;
    }
    const reference back() const {
        const_iterator _tmp = end();
        --_tmp;
        return *_tmp;
    }


    /// iterator
    iterator begin() { return this->_data._start; }
    const_iterator cbegin() const { return this->_data._start; }
    iterator end() { return this->_data._finish; }
    const_iterator cend() const { return this->_data._finish; }


    /// capacity
    size_type size() const { return this->_data._finish - this->_data._start; }
    bool empty() const { return this->_data._finish == this->_data._start; }
};

};

#endif