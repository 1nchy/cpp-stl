#ifndef  _ASP_DEQUE_HPP_
#define _ASP_DEQUE_HPP_

#include <ostream>

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
    void shrink_to_fit();


    /// modifiers
    void push_back(const value_type& _x) {
        if (this->_data._finish._cur != this->_data._finish._last - 1) {
            this->_M_construct_node(this->_data._finish._cur, _x);
            ++this->_data._finish;
        }
        else {
            _M_alloc_push_back(_x);
        }
    }
    void push_back(value_type&& _x) {
        emplace_back(std::move(_x));
    }
    void pop_back() {
        if (this->_data._finish._cur != this->_data._finish._first) {
            --this->_data._finish._cur;
            this->_M_destory_node(this->_data._finish_cur);
        }
        else {

        }
    }
    void push_front(const value_type& _x) {}
    void push_front(value_type&& _x) {
        emplace_front(std::move(_x));
    }
    void pop_front() {}
    template <typename... _Args> iterator emplace(const_iterator _pos, _Args&&... _args) {}
    template <typename... _Args> void emplace_back(_Args&&... _args) {}
    template <typename... _Args> void emplace_front(_Args&&... _args) {}
    iterator insert(const_iterator _pos, const value_type& _e) {}
    template <typename _InputIterator> iterator insert(const_iterator _pos, _InputIterator _first, _InputIterator _last) {}
    iterator erase(const_iterator _pos) {}
    iterator erase(const_iterator _first, const_iterator _last) {}
    void clear() {}


    /// ostream
    template <typename _R> friend std::ostream& operator<<(std::ostream& os, const deque<_R>& d) {
        os << '[';
        for (auto p = d.cbegin(); p != d.cend(); ++p) {
            os << *p;
            if (p + 1 != d.cend()) {
                os << ", ";
            }
        }
        os << ']';
        return os;
    }


protected:
    // to make sure at least %_nodes_to_add nodes exist at back
    void _M_reserve_map_at_back(size_type _nodes_to_add = 1) {
        if (_nodes_to_add + 1 > size_type(this->_data._map_size - (this->_data._finish._node - this->_data._map))) {
            _M_reallocate_map();
        }
    }
    // to make sure at least %_nodes_to_add nodes exist at front
    void _M_reserve_map_at_front(size_type _nodes_to_add = 1) {
        if (_nodes_to_add > size_type(this->_data._start._node - this->_data._map)) {
            _M_reallocate_map();
        }
    }
    void _M_reallocate_map() {

    }

    template <typename... _Args> void _M_alloc_push_back(_Args&&... _args) {

    }
    
};

};

/*
map memory model:
    [%_map] [%_start] ... [%_finish] [] (%_map + %_map_size)
    each one is a node, which contains %data_type::_S_buffer_size() elements

iterator & node memory model:
    node => [%_first] [%_cur] ... [] (%_last)
    %iterator::_node => node
*/

#endif