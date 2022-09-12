#ifndef _ASP_VECTOR_HPP_
#define _ASP_VECTOR_HPP_

#include <ostream>
#include <iostream>

#include "vector_base.hpp"
#include "iterator.hpp"

namespace asp {
template <typename _Tp> struct vector_data;
template <typename _Tp, typename _Alloc> struct vector_impl;
template <typename _Tp, typename _Alloc> struct vector_base;
template <typename _Tp, typename _Alloc> class vector;

template <typename _Tp, typename _Alloc = std::allocator<_Tp>> class vector : public vector_base<_Tp, _Alloc> {
public:
    typedef _Tp value_type;
    typedef vector_base<_Tp, _Alloc> base;
    typedef vector<_Tp, _Alloc> self;
    typedef typename base::pointer pointer;
    typedef typename base::reference reference;
    typedef _Alloc allocator_type;
    typedef normal_iterator<value_type> iterator;
    typedef normal_iterator<const value_type> const_iterator;
    typedef typename base::alloc_traits alloc_traits;

    using base::m_data;

    vector() = default;
    explicit vector(const allocator_type& _a) : base(_a) {}
    explicit vector(size_type _n, const allocator_type& _a = allocator_type()) : base(_n, _a) {}
    vector(const vector& _x) : base(_x.size(), _x.get_allocator()) {
        // m_data.copy(_x.m_data);
        this->m_data.finish = std::uninitialized_copy(_x.m_data.start, _x.m_data.finish, this->m_data.finish);
        this->m_data.end_of_storage = this->m_data.start + _x.size();
    }
    vector(vector&& _x) : base(_x.size(), _x.get_allocator()) {
        // m_data.swap(std::move(_x).m_data);
        this->m_data.finish = std::uninitialized_copy(std::move(_x).m_data.start, std::move(_x).m_data.finish, this->m_data.finish);
        this->m_data.end_of_storage = this->m_data.start + _x.size();
    }
    // ~vector() {
    //     std::_Destroy(this->m_data.start, this->m_data.finish);
    //     this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
    // }
    virtual ~vector() = default;


    /// iterator
    iterator begin() { return iterator(m_data.start); }
    iterator end() { return iterator(m_data.finish); }
    const_iterator cbegin() const { return const_iterator(m_data.start); }
    const_iterator cend() const {return const_iterator(m_data.finish); }

    /// capacity
    size_type size() const {
        return this->m_data.finish - this->m_data.start;
    }
    size_type capacity() const {
        return this->m_data.end_of_storage - this->m_data.start;
    }
    bool empty() const { return m_data.start == m_data.finish; }
    void reserve(size_type _n) {
        if (_n > this->capacity()) {
            const size_type _old_size = size();
            pointer _tmp = _M_allocate_copy(_n, this->begin(), this->end());
            std::_Destroy(this->m_data.start, this->m_data.finish);
            this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
            this->m_data.start = _tmp;
            this->m_data.finish = _tmp + _old_size;
            this->m_data.end_of_storage = _tmp + _n;
        }
    }
    void shrink_to_fit() {
        if (size() == capacity()) {
            return;
        }
        const size_type _n = this->size();
        pointer _new_start = _M_allocate_copy(_n, this->m_data.start, this->m_data.finish);
        std::_Destroy(this->m_data.start, this->m_data.finish);
        this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
        this->m_data.start = _new_start;
        this->m_data.finish = _new_start + _n;
        this->m_data.end_of_storage = this->m_data.finish;
    }

    /// element access
    // value_type& at(size_type _n);
    // const value_type& at(size_type _n) const;
    value_type& operator[] (size_type _n) {
        return *(m_data.start + _n);
    }
    const value_type& operator[] (size_type _n) const {
        return *(m_data.start + _n);
    }
    value_type& front() { return *(m_data.start); }
    const value_type& front() const { return *(m_data.start); }
    value_type& back() { return *(m_data.end - 1); }
    const value_type& back() const { return *(m_data.end - 1); }
    pointer data() { return m_data.start; }
    const pointer data() const { return m_data.start; }

    /// modifiers
    void push_back(const value_type& _x) {
        if (m_data.finish != m_data.end_of_storage) {
            alloc_traits::construct(this->m_data, this->m_data.finish, _x);
            ++this->m_data.finish;
        }
        else {
            _M_realloc_insert(end(), _x);
        }
    }
    void push_back(value_type&& _x) {
        emplace_back(std::move(_x));
    }
    void pop_back() {
        --this->m_data.finish;
        alloc_traits::destroy(this->m_data, this->m_data.finish);
    }
    iterator insert(const_iterator _pos, const value_type& _e) {
        const size_type _n = _pos - cbegin();
        if (this->m_data.finish != this->m_data.end_of_storage) {
            if (_pos == cend()) {
                _M_construct(begin() + _n, _e);
                ++this->m_data.finish;
            }
            else {
                _M_insert(begin() + _n, _e);
            }
        }
        else {
            _M_realloc_insert(begin() + _n, _e);
        }
        return iterator(this->m_data.start + _n);
    }
    template <typename _InputIterator> iterator insert(const_iterator _pos, _InputIterator _first, _InputIterator _last) {
        const size_type _offset = _pos - cbegin();
    }
    iterator erase(const_iterator _pos) {}
    iterator erase(const_iterator _first, const_iterator _last) {}
    void clear() {
        std::_Destroy(this->m_data.start, this->m_data.finish);
        this->m_data.finish = this->m_data.start;
    }
    template <class... _Args> iterator emplace(const_iterator _pos, _Args&&... _args) {
        return _M_emplace(_pos, std::forward<_Args>(_args)...);
    }
    template <class... _Args> void emplace_back(_Args&&... _args) {
        if (m_data.finish != m_data.end_of_storage) {
            alloc_traits::construct(this->m_data, this->m_data.finish, std::forward<_Args>(_args)...);
            ++this->m_data.finish;
        }
        else {
            _M_realloc_insert(end(), std::forward<_Args>(_args)...);
        }
    }

    /// allocator
    allocator_type get_allocator() const {
        return allocator_type(this->m_data);
    }

    /// ostream
    template <typename _R> friend std::ostream& operator<<(std::ostream& os, const vector<_R>& v) {
        os << '[';
        for (auto p = v.cbegin(); p != v.cend(); ++p) {
            os << *p;
            if (p + 1 != v.cend()) {
                os << ", ";
            }
        }
        os << ']';
        return os;
    }
protected:
    void _M_construct(iterator _pos, const value_type& _x) {
        alloc_traits::construct(this->m_data, _pos._ptr, _x);
    }
    template <class... _Args> void _M_construct(iterator _pos, _Args&&... _args) {
        alloc_traits::construct(this->m_data, _pos._ptr, std::forward<_Args>(_args)...);
    }
    void _M_destory(iterator _pos) {
        alloc_traits::destory(this->m_data, _pos._ptr);
    }
    pointer _M_allocate_copy(size_type _n, iterator first, iterator last) {
        pointer _ret = this->_M_allocate(_n);
        std::uninitialized_copy(first._ptr, last._ptr, _ret);
        return _ret;
    }
    size_type _M_realloc_size(size_type _n) const {
        const size_type _len = size() + std::max(size(), _n);
        return _len;
    }
    // need to realloc when inserting
    void _M_realloc_insert(iterator _pos, const value_type& _x) {
        const size_type _len = _M_realloc_size(size_type(1));
        const size_type size_before = _pos - begin();
        pointer _new_start(this->_M_allocate(_len));
        pointer _new_finish(_new_start);

        _M_construct(_new_start + size_before, _x);
        _new_finish = std::uninitialized_copy(this->m_data.start, _pos._ptr, _new_finish);
        ++_new_finish;
        _new_finish = std::uninitialized_copy(_pos._ptr, this->m_data.finish, _new_finish);

        std::_Destroy(this->m_data.start, this->m_data.finish);
        this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
        this->m_data.start = _new_start;
        this->m_data.finish = _new_finish;
        this->m_data.end_of_storage = _new_start + _len;
    }
    template <class... _Args> void _M_realloc_insert(iterator _pos, _Args&&... _args) {
        const size_type _len = _M_realloc_size(size_type(1));
        const size_type size_before = _pos - begin();
        pointer _new_start(this->_M_allocate(_len));
        pointer _new_finish(_new_start);

        _M_construct(_new_start + size_before, std::forward<_Args>(_args)...);
        _new_finish = std::uninitialized_copy(this->m_data.start, _pos._ptr, _new_finish);
        ++_new_finish;
        _new_finish = std::uninitialized_copy(_pos._ptr, this->m_data.finish, _new_finish);

        std::_Destroy(this->m_data.start, this->m_data.finish);
        this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
        this->m_data.start = _new_start;
        this->m_data.finish = _new_finish;
        this->m_data.end_of_storage = _new_start + _len;
    }
    // no need to realloc when inserting
    void _M_insert(iterator _pos, const value_type& _x) {
        _M_construct(this->m_data.finish, *(this->m_data.finish - 1));
        ++this->m_data.finish;
        value_type _x_copy = _x;
        std::copy_backward(_pos._ptr, this->m_data.finish - 2,
                            this->m_data.finish - 1);
        *_pos = _x_copy;
    }
    template <class... _Args> void _M_insert(iterator _pos, _Args&&... _args) {
        _M_construct(this->m_data.finish, *(this->m_data.finish - 1));
        ++this->m_data.finish;
        std::copy_backward(_pos._ptr, this->m_data.finish - 2,
                            this->m_data.finish - 1);
        _M_construct(_pos, std::forward<_Args>(_args)...);
    }
    template <class... _Args> iterator _M_emplace(const_iterator _pos, _Args&&... _args) {
        const size_type _n = _pos - cbegin();
        if (this->m_data.finish != this->m_data.end_of_storage) {
            if (_pos == cend()) {
                _M_construct(begin() + _n, std::forward<_Args>(_args)...);
                ++this->m_data.finish;
            }
            else {
                _M_insert(begin() + _n, std::forward<_Args>(_args)...);
            }
        }
        else {
            _M_realloc_insert(begin() + _n, std::forward<_Args>(_args)...);
        }
        return iterator(this->m_data.start + _n);
    }
};

};

#endif