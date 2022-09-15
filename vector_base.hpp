#ifndef _ASP_VECTOR_BASE_HPP_
#define _ASP_VECTOR_BASE_HPP_

#include <memory>

#include "basic_param.hpp"

namespace asp {
template <typename _Tp> struct vector_data;
template <typename _Tp, typename _Alloc> struct vector_impl;
template <typename _Tp, typename _Alloc> struct vector_base;

template <typename _Tp> struct vector_data {
    typedef _Tp value_type;
    typedef _Tp* pointer;
    typedef vector_data<value_type> vdata;

    vector_data() {}
    vector_data(const vdata& rhs):
        start(rhs.start), finish(rhs.finish), end_of_storage(rhs.end_of_storage) {}
    vector_data(vdata&& rhs):
        start(rhs.start), finish(rhs.finish), end_of_storage(rhs.end_of_storage) {
            rhs.reset();
        }
    vdata& operator=(const vdata& rhs) {
        start = rhs.start; finish = rhs.finish; end_of_storage = rhs.end_of_storage;
    }
    vdata& operator=(vdata&& rhs) {
        start = rhs.start; finish = rhs.finish; end_of_storage = rhs.end_of_storage;
        rhs.reset();
    }


    void copy(const vdata& d) {
        start = d.start; finish = d.finish; end_of_storage = d.end_of_storage;
    }
    void swap(vdata& d) {
        vdata _tmp(*this);
        this->copy(d);
        d->copy(_tmp);
    }
    void reset() {
        start = nullptr; finish = nullptr; end_of_storage = nullptr;
    }

public:
    pointer start = nullptr;
    pointer finish = nullptr;
    pointer end_of_storage = nullptr;
};


template <typename _Tp, class _Alloc = std::allocator<_Tp>> struct vector_impl
 : public vector_data<_Tp>, public _Alloc {
    typedef _Alloc allocator_type;
    typedef vector_data<_Tp> base;
    typedef vector_impl<_Tp, _Alloc> self;
    typedef _Tp value_type;
    typedef _Tp* pointer;

    vector_impl() : _Alloc(), base() {}
    explicit vector_impl(const allocator_type& a) : _Alloc(a), base() {}
    vector_impl(vector_impl&& x) : _Alloc(std::move(x)), base(x) {}

    using base::start;
    using base::finish;
    using base::end_of_storage;
};

template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct vector_base {
    typedef _Tp value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef _Alloc allocator_type;
    typedef vector_base<value_type, allocator_type> self;
    typedef std::allocator_traits<_Alloc> alloc_traits;

    vector_base() = default;
    explicit vector_base(const allocator_type& _a) : m_data(_a) {}
    vector_base(size_type _n) : m_data() {
        _M_create_storage(_n);
    }
    explicit vector_base(size_type _n, const allocator_type& _a) : m_data(_a) {
        _M_create_storage(_n);
    }
    vector_base(self&& _vb) = default;
    virtual ~vector_base() {
        std::_Destroy(this->m_data.start, this->m_data.finish);
        this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
    }

    void _M_construct(pointer _p, const value_type& _x) {
        alloc_traits::construct(this->m_data, _p, _x);
    }
    template <class... _Args> void _M_construct(pointer _p, _Args&&... _args) {
        alloc_traits::construct(this->m_data, _p, std::forward<_Args>(_args)...);
    }
    void _M_destory(pointer _p) {
        alloc_traits::destory(this->m_data, _p);
    }
    void _M_destory(pointer _first, pointer _last) {
        std::_Destroy(_first, _last, this->m_data);
    }
    pointer _M_allocate(size_type _n) {
        return _n != 0 ? alloc_traits::allocate(m_data, _n) : pointer();
    }
    void _M_deallocate(pointer _p, size_type _n) {
        if (_p != nullptr) {
            alloc_traits::deallocate(m_data, _p, _n);
        }
    }

    void _M_create_storage(size_type _n) {
        this->m_data.start = _M_allocate(_n);
        this->m_data.finish = this->m_data.start;
        this->m_data.end_of_storage = this->m_data.start + _n;
    }

public:
    vector_impl<value_type, allocator_type> m_data;
};

};

#endif