#ifndef  _ASP_DEQUE_BASE_HPP_
#define _ASP_DEQUE_BASE_HPP_

#include "basic_param.hpp"

#include <memory>

namespace asp {

template <typename _Tp> struct deque_data;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct deque_impl;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct deque_base;

template <typename _Tp> struct deque_iterator;

template <typename _Tp> struct deque_data {
    typedef _Tp value_type;
    typedef _Tp* elt_pointer;
    typedef _Tp** map_pointer;

    map_pointer _map = nullptr;
    size_type _map_size = 0;
    elt_pointer _start = nullptr;
    elt_pointer _finish = nullptr;

    deque_data() = default;
    deque_data(deque_data&& _d) = default;
    virtual ~deque_data() = default;
};

template <typename _Tp, typename _Alloc> struct deque_impl
 : public deque_data<_Tp>, public _Alloc {
    typedef deque_data<_Tp> base;
    typedef deque_impl<_Tp, _Alloc> self;
    using base::value_type;
    using base::elt_pointer;
    using base::map_pointer;

    typedef _Alloc elt_allocator_type;
    typedef std::allocator_traits<elt_allocator_type> elt_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<elt_pointer> map_allocator_type;
    typedef std::allocator_traits<map_allocator_type> map_alloc_traits;

    deque_impl() : _Alloc(), base() {}
    explicit deque_impl(const elt_allocator_type& a) : _Alloc(a), base() {}
    explicit deque_impl(elt_allocator_type&& a) : _Alloc(std::move(a)), base() {}
    deque_impl(deque_impl&& x) : _Alloc(std::move(x)), base(std::move(x)) {}
    virtual ~deque_impl() = default;
    
    using base::_map;
    using base::_map_size;
    using base::_start;
    using base::_finish;
};

template <typename _Tp, typename _Alloc> struct deque_base {
    typedef _Alloc elt_allocator_type;
    typedef deque_base<_Tp, _Alloc> self;
    typedef deque_impl<value_type, elt_allocator_type> data_type;
    typedef std::allocator_traits<_Alloc> elt_alloc_traits;
    typedef typename data_type::value_type value_type;
    
    
    deque_base() : _data() { _M_initialize_map(0); }
    deque_base(size_type _num_elts) : _data() { _M_initialize_map(_num_elts); }
    deque_base(const elt_allocator_type& _a, size_type _num_elts = 0) : _data(_a) { _M_initialize_map(_num_elts); }
    deque_base(self&& _x) : _data(_x._M_move_data()) {}
    deque_base(self&& _x, const elt_allocator_type& _a, size_type _n) : _data(_x._M_move_data()) {
        // if (_x.)
    }




    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type>(&this->_data); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<elt_allocator_type>(&this->_data); }
    void _M_initialize_map(size_type _num_elts) {

    }

    data_type _data;
private:
    data_type _M_move_data() {

    }
};

template <typename _Tp> struct deque_iterator {

};

};

#endif