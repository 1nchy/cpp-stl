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
        this->m_data.finish = _M_uninitialized_copy(_x.m_data.start, _x.m_data.finish, this->m_data.finish);
        this->m_data.end_of_storage = this->m_data.start + _x.size();
    }
    vector(vector&& _x) : base(_x.size(), _x.get_allocator()) {
        // m_data.swap(std::move(_x).m_data);
        this->m_data.finish = _M_uninitialized_copy(std::move(_x).m_data.start, std::move(_x).m_data.finish, this->m_data.finish);
        this->m_data.end_of_storage = this->m_data.start + _x.size();
    }
    self& operator=(const vector& _x) {
        if (&_x == this) return *this;
        const size_type _xlen = _x.size();
        if (_xlen > capacity()) {
            pointer _tmp = _M_allocate_copy(_xlen, _x.begin(), _x.end());
            this->_M_destory(this->m_data.start, this->m_data.finish);
            this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
            this->m_data.start = _tmp;
            this->m_data.end_of_storage = this->m_data.start + _xlen;
        }
        else {
            this->_M_destory(this->m_data.start, this->m_data.finish);
            this->_M_uninitialized_copy(_x.m_data.start, _x.m_data.finish, this->m_data.start);
        }
        this->m_data.finish = this->m_data.start + _xlen;
        return *this;
    }
    // ~vector() {
    //     this->_M_destory(this->m_data.start, this->m_data.finish);
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
            pointer _tmp = _M_allocate_copy(_n, this->m_data.start, this->m_data.finish);
            this->_M_destory(this->m_data.start, this->m_data.finish);
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
        this->_M_destory(this->m_data.start, this->m_data.finish);
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
            _M_realloc_insert(this->m_data.finish, _x);
        }
    }
    void push_back(value_type&& _x) {
        emplace_back(std::move(_x));
    }
    void pop_back() {
        if (empty()) { return; }
        --this->m_data.finish;
        alloc_traits::destroy(this->m_data, this->m_data.finish);
    }
    iterator insert(const_iterator _pos, const value_type& _e) {
        const size_type _n = _pos - cbegin();
        if (this->m_data.finish != this->m_data.end_of_storage) {
            if (_pos == cend()) {
                this->_M_construct(this->m_data.start + _n, _e);
                ++this->m_data.finish;
            }
            else {
                _M_insert(this->m_data.start + _n, _e);
            }
        }
        else {
            _M_realloc_insert(this->m_data.start + _n, _e);
        }
        return iterator(this->m_data.start + _n);
    }
    template <typename _InputIterator> iterator insert(const_iterator _pos, _InputIterator _first, _InputIterator _last) {
        const size_type _offset = _pos - cbegin();
        _M_range_insert(this->m_data.start + _offset, _first, _last);
        return begin() + _offset;
    }
    iterator erase(const_iterator _pos) {
        if (_pos == cend()) return end();
        return this->_M_erase(begin() + (_pos - cbegin()));
    }
    iterator erase(const_iterator _first, const_iterator _last) {
        const auto _beg = begin();
        const auto _cbeg = cbegin();
        return this->_M_range_erase(_beg + (_first - _cbeg), _beg + (_last - _cbeg));
    }
    void clear() {
        this->_M_destory(this->m_data.start, this->m_data.finish);
        this->m_data.finish = this->m_data.start;
    }
    template <class... _Args> iterator emplace(const_iterator _pos, _Args&&... _args) {
        return _M_emplace(_pos.base(), std::forward<_Args>(_args)...);
    }
    template <class... _Args> void emplace_back(_Args&&... _args) {
        if (m_data.finish != m_data.end_of_storage) {
            alloc_traits::construct(this->m_data, this->m_data.finish, std::forward<_Args>(_args)...);
            ++this->m_data.finish;
        }
        else {
            _M_realloc_insert(this->m_data.finish, std::forward<_Args>(_args)...);
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
            os << p;
            if (p + 1 != v.cend()) {
                os << ", ";
            }
        }
        os << ']';
        return os;
    }
protected:
    pointer _M_uninitialized_copy(pointer _first, pointer _last, pointer _res) {
        return std::__uninitialized_copy_a(_first, _last, _res, this->m_data);
    }
    pointer _M_uninitialized_move(pointer _first, pointer _last, pointer _res) {
        return std::__uninitialized_move_a(_first, _last, _res, this->m_data);
    }
    pointer _M_allocate_copy(size_type _n, pointer _first, pointer _last) {
        pointer _ret = this->_M_allocate(_n);
        _M_uninitialized_copy(_first, _last, _ret);
        return _ret;
    }
    size_type _M_realloc_size(size_type _n) const {
        const size_type _len = size() + std::max(size(), _n);
        return _len;
    }
    // need to realloc when inserting
    void _M_realloc_insert(pointer _p, const value_type& _x) {
        const size_type _len = _M_realloc_size(size_type(1));
        const size_type size_before = _p - this->m_data.start;
        pointer _new_start(this->_M_allocate(_len));
        pointer _new_finish(_new_start);

        this->_M_construct(_new_start + size_before, _x);
        _new_finish = _M_uninitialized_copy(this->m_data.start, _p, _new_finish);
        ++_new_finish;
        _new_finish = _M_uninitialized_copy(_p, this->m_data.finish, _new_finish);

        this->_M_destory(this->m_data.start, this->m_data.finish);
        this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
        this->m_data.start = _new_start;
        this->m_data.finish = _new_finish;
        this->m_data.end_of_storage = _new_start + _len;
    }
    template <class... _Args> void _M_realloc_insert(pointer _p, _Args&&... _args) {
        const size_type _len = _M_realloc_size(size_type(1));
        const size_type size_before = _p - this->m_data.start;
        pointer _new_start(this->_M_allocate(_len));
        pointer _new_finish(_new_start);

        this->_M_construct(_new_start + size_before, std::forward<_Args>(_args)...);
        _new_finish = _M_uninitialized_copy(this->m_data.start, _p, _new_finish);
        ++_new_finish;
        _new_finish = _M_uninitialized_copy(_p, this->m_data.finish, _new_finish);

        this->_M_destory(this->m_data.start, this->m_data.finish);
        this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
        this->m_data.start = _new_start;
        this->m_data.finish = _new_finish;
        this->m_data.end_of_storage = _new_start + _len;
    }
    // no need to realloc when inserting
    void _M_insert(pointer _p, const value_type& _x) {
        this->_M_construct(this->m_data.finish, *(this->m_data.finish - 1));
        ++this->m_data.finish;
        value_type _x_copy = _x;
        std::move_backward(_p, this->m_data.finish - 2,
                            this->m_data.finish - 1);
        *_p = _x_copy;
    }
    template <class... _Args> void _M_insert(pointer _p, _Args&&... _args) {
        this->_M_construct(this->m_data.finish, *(this->m_data.finish - 1));
        ++this->m_data.finish;
        std::move_backward(_p, this->m_data.finish - 2,
                            this->m_data.finish - 1);
        this->_M_construct(_p, std::forward<_Args>(_args)...);
    }
    template <typename _Iter> void _M_range_insert(pointer _p, _Iter _first, _Iter _last) {
        if (_first == _last) {
            return;
        }
        const size_type _n = asp::distance(_first, _last);
        pointer _old_finish(this->m_data.finish);
        if (size_type(this->m_data.end_of_storage - this->m_data.finish) >= _n) {
            // allocated memory is enough
            const size_type _elems_after = this->m_data.finish - _p;
            if (_elems_after > _n) {
                _M_uninitialized_move(this->m_data.finish - _n, this->m_data.finish, this->m_data.finish);
                this->m_data.finish += _n;
                std::move_backward(_p, _old_finish - _n, _old_finish);
                // std::copy(_first, _last, iterator(_p));
                std::__uninitialized_copy_a(_first, _last, _p, this->m_data);
            }
            else {
                _Iter _mid = _first;
                asp::advance(_mid, _elems_after);
                // _M_uninitialized_copy(_mid.base(), _last.base(), this->m_data.finish);
                std::__uninitialized_copy_a(_mid, _last, this->m_data.finish, this->m_data);
                this->m_data.finish += _n - _elems_after;
                _M_uninitialized_move(_p, _old_finish, this->m_data.finish);
                this->m_data.finish += _elems_after;
                // std::copy(_first, _mid, iterator(_p));
                std::__uninitialized_copy_a(_first, _mid, _p, this->m_data);
            }
        }
        else {
            const size_type _len = _M_realloc_size(_n);
            pointer _new_start(this->_M_allocate(_len));
            pointer _new_finish(_new_start);
            _new_finish = _M_uninitialized_move(this->m_data.start, _p, _new_finish);
            // _new_finish = _M_uninitialized_copy(_first.base(), _last.base(), _new_finish); /// caution!!
            _new_finish = std::__uninitialized_copy_a(_first, _last, _new_finish, this->m_data);
            _new_finish = _M_uninitialized_move(_p, this->m_data.finish, _new_finish);
            this->_M_destory(this->m_data.start, this->m_data.finish);
            this->_M_deallocate(this->m_data.start, this->m_data.end_of_storage - this->m_data.start);
            this->m_data.start = _new_start;
            this->m_data.finish = _new_finish;
            this->m_data.end_of_storage = _new_start + _len;
        }
    }
    iterator _M_erase(iterator _pos) {
        if (_pos + 1 != end()) {
            std::move(_pos + 1, end(), _pos);
        }
        --this->m_data.finish;
        this->_M_destory(this->m_data.finish);
        return _pos;
    }
    iterator _M_range_erase(iterator _first, iterator _last) {
        if (_first != _last) {
            if (_last != end()) {
                std::move(_last, end(), _first);
            }
            iterator _pos = _first + (end() - _last);
            this->_M_destory(_pos.base(), this->m_data.finish);
            this->m_data.finish = _pos.base();
        }
        return _first;
    }
    template <class... _Args> iterator _M_emplace(const pointer _p, _Args&&... _args) {
        const size_type _n = _p - this->m_data.start;
        if (this->m_data.finish != this->m_data.end_of_storage) {
            if (_p == this->m_data.finish) {
                this->_M_construct(this->m_data.start + _n, std::forward<_Args>(_args)...);
                ++this->m_data.finish;
            }
            else {
                _M_insert(this->m_data.start + _n, std::forward<_Args>(_args)...);
            }
        }
        else {
            _M_realloc_insert(this->m_data.start + _n, std::forward<_Args>(_args)...);
        }
        return iterator(this->m_data.start + _n);
    }
};

};

#endif