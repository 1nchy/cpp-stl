#ifndef  _ASP_DEQUE_BASE_HPP_
#define _ASP_DEQUE_BASE_HPP_

#include "basic_param.hpp"

#include <memory>

namespace asp {

#ifndef _ASP_DEQUE_BUF_SIZE
// Bit size of one node
#define _ASP_DEQUE_BUF_SIZE 512
#endif

template <typename _Tp> struct deque_data;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct deque_impl;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct deque_base;

// not only an iterator, but also sentinel of deque_base
template <typename _Tp, typename _Ref = _Tp&, typename _Ptr = _Tp*> struct deque_iterator;

inline size_type deque_buf_size(size_type _size) {
    return _size < _ASP_DEQUE_BUF_SIZE ?
        size_type(_ASP_DEQUE_BUF_SIZE / _size) :
        size_type(1);
}

template <typename _Tp, typename _Ref, typename _Ptr> struct deque_iterator {
    typedef deque_data<_Tp> data_type;
    using value_type = typename data_type::value_type;
    using pointer = typename data_type::pointer;
    using reference = typename data_type::reference;
    using elt_pointer = typename data_type::elt_pointer;
    using map_pointer = typename data_type::map_pointer;

    using iterator = typename data_type::iterator;
    using const_iterator = typename data_type::const_iterator;

    typedef deque_iterator self;

    elt_pointer _cur;
    elt_pointer _first;
    elt_pointer _last;
    map_pointer _node;

    deque_iterator()
     : _cur(), _first(), _last(), _node() {};
    deque_iterator(elt_pointer _x, map_pointer _y)
     : _cur(_x), _first(*_y), _last(*_y + data_type::_S_buffer_size()), _node(_y) {}
    // deque_iterator(const deque_iterator& _x)
    //  : _cur(_x._cur), _first(_x._first), _last(_x._last), _node(_x._node) {}
    self& operator=(const deque_iterator& _x) {
        _cur = _x._cur; _first = _x._first; _last = _x._last; _node = _x._node;
        return *this;
    }
    // conversion from iterator to const_iterator
    deque_iterator(const iterator& _x)
     : _cur(_x._cur), _first(_x._first), _last(_x._last), _node(_x._node) {}

    iterator _M_const_cast() const { return iterator(_cur, _node); }
    reference operator*() const { return *_cur; }
    pointer operator->() const { return _cur; }
    self& operator++() {
        ++_cur;
        if (_cur == _last) {
            _M_set_node(_node + 1);
            _cur = _first;
        }
        return *this;
    }
    self operator++(int) {
        self _tmp = *this;
        ++*this;
        return _tmp;
    }
    self& operator--() {
        if (_cur == _first) {
            _M_set_node(_node - 1);
            _cur = _last;
        }
        --_cur;
        return *this;
    }
    self operator--(int) {
        self _tmp = *this;
        --*this;
        return _tmp;
    }
    self& operator+=(difference_type _n) {
        const difference_type _offset = _n + (_cur - _first);
        if (_offset >= 0 && _offset < difference_type(data_type::_S_buffer_size())) {
            _cur += _n;
        }
        else {
            const difference_type _node_offset = _offset > 0 ?
                _offset / difference_type(data_type::_S_buffer_size()) :
                -difference_type((_offset - 1) / data_type::_S_buffer_size()) - 1;
            _M_set_node(_node + _node_offset);
            _cur = _first + (_offset - _node_offset * difference_type(data_type::_S_buffer_size()));
        }
        return *this;
    }
    self& operator-=(difference_type _n) {
        return *this += -_n;
    }
    friend bool operator==(const self& _x, const self& _y) {
        return _x._cur == _y._cur;
    }
    template <typename _RefR, typename _PtrR> friend bool
     operator==(const self& _x, const deque_iterator<_Tp, _RefR, _PtrR>& _y) {
        return _x._cur == _y._cur;
    }
    friend bool operator!=(const self& _x, const self& _y) {
        return !(_x == _y);
    }
    template <typename _RefR, typename _PtrR> friend bool
     operator!=(const self& _x, const deque_iterator<_Tp, _RefR, _PtrR>& _y) {
        return !(_x == _y);
    }
    
    friend difference_type operator-(const self& _x, const self& _y) {
        return difference_type(data_type::_S_buffer_size()) * (_x._node - _y._node - 1)
         + (_x._cur - _x._first) + (_y._last - _y._cur);
    }
    friend self operator-(const self& _x, difference_type _n) {
        self _tmp = _x;
        _tmp -= _n;
        return _tmp;
    }
    friend self operator+(const self& _x, difference_type _n) {
        self _tmp = _x;
        _tmp += _n;
        return _tmp;
    }
    friend self operator+(difference_type _n, const self& _x) {
        return _x + _n;
    }

    void _M_set_node(map_pointer _new_node) {
        _node = _new_node;
        _first = *_node;
        _last = _first + difference_type(data_type::_S_buffer_size());
    }
};

template <typename _Tp> struct deque_data {
    typedef _Tp value_type;
    typedef _Tp* pointer;
    typedef _Tp& reference;
    typedef _Tp* elt_pointer;
    typedef _Tp** map_pointer;
    typedef deque_iterator<_Tp, _Tp&, _Tp*> iterator;
    typedef deque_iterator<_Tp, const _Tp&, const _Tp*> const_iterator;

    typedef deque_data<_Tp> self;

    map_pointer _map;
    size_type _map_size = 0;
    iterator _start;
    iterator _finish;

    deque_data() : _map(), _map_size(0), _start(), _finish() {};
    deque_data(self&& _d) = default;
    virtual ~deque_data() = default;

    constexpr static size_type _S_buffer_size() {
        return deque_buf_size(sizeof(_Tp));
    }

    void swap_data(self&& _x) {
        using std::swap;
        swap(this->_start, _x._start);
        swap(this->_finish, _x._finish);
        swap(this->_map, _x._map);
        swap(this->_map_size, _x._map_size);
    }
};

template <typename _Tp, typename _Alloc> struct deque_impl
 : public deque_data<_Tp>, public _Alloc {
    typedef deque_data<_Tp> base;
    typedef deque_impl<_Tp, _Alloc> self;
    using value_type = typename base::value_type;
    using elt_pointer = typename base::elt_pointer;
    using map_pointer = typename base::map_pointer;
    using iterator = typename base::iterator;
    using const_iterator = typename base::const_iterator;

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
    using base::swap_data;
    using base::_S_buffer_size;
};

template <typename _Tp, typename _Alloc> struct deque_base {
    typedef deque_base<_Tp, _Alloc> self;
    typedef deque_impl<_Tp, _Alloc> data_type;
    typedef typename data_type::value_type value_type;
    typedef typename data_type::elt_pointer elt_pointer;
    typedef typename data_type::map_pointer map_pointer;
    typedef typename data_type::elt_allocator_type elt_allocator_type;
    typedef typename data_type::elt_alloc_traits elt_alloc_traits;
    typedef typename data_type::map_allocator_type map_allocator_type;
    typedef typename data_type::map_alloc_traits map_alloc_traits;
    
    deque_base() : _data() { _M_initialize_map(0); }
    deque_base(size_type _num_elts) : _data() { _M_initialize_map(_num_elts); }
    deque_base(const elt_allocator_type& _a, size_type _num_elts = 0) : _data(_a) { _M_initialize_map(_num_elts); }
    deque_base(self&& _x) : _data(_x._M_move_data()) {}
    deque_base(self&& _x, const elt_allocator_type& _a, size_type _n) : _data(_x._M_move_data()) {
        if (_x._M_get_elt_allocator() == _a) {
            if (_x._data._map) {
                _M_initialize_map(0);
                this->_data.swap_data(_x._data);
            }
        }
        else {
            _M_initialize_map(_n);
        }
    }
    ~deque_base() {
        if (this->_data._map) {
            // _M_destory_nodes(this->_data._start)
        }
    }
    constexpr static const size_type _S_initial_map_size = 8;


    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type*>(&this->_data); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<const elt_allocator_type*>(&this->_data); }
    map_allocator_type _M_get_map_allocator() const { return map_allocator_type(_M_get_elt_allocator()); }

    // node is a basic storage unit of @deque
    elt_pointer _M_allocate_node() {
        return elt_alloc_traits::allocate(_data, deque_buf_size(sizeof(value_type)));
    }
    void _M_deallocate_node(elt_pointer _p) {
        elt_alloc_traits::deallocate(_data, _p, deque_buf_size(sizeof(value_type)));
    }
    map_pointer _M_allocate_map(size_type _n) {
        map_allocator_type _map_data = _M_get_map_allocator();
        return map_alloc_traits::allocate(_map_data, _n);
    }
    void _M_deallocate_map(map_pointer _p, size_type _n) {
        map_allocator_type _map_data = _M_get_map_allocator();
        map_alloc_traits::deallocate(_map_data, _p, _n);
    }

    void _M_initialize_map(size_type _num_elts) {
        const size_type _num_nodes = (_num_elts / data_type::_S_buffer_size()) + 1;
        this->_data._map_size = std::max(self::_S_initial_map_size, _num_nodes + 2);
        this->_data._map = _M_allocate_map(this->_data._map_size);

        map_pointer _nstart = this->_data._map + (this->_data._map_size - _num_nodes) / 2;
        map_pointer _nfinish = _nstart + _num_nodes;

        _M_create_nodes(_nstart, _nfinish);

        this->_data._start._M_set_node(_nstart);
        this->_data._finish._M_set_node(_nfinish - 1);
        this->_data._start._cur = this->_data._start._first;
        this->_data._finish._cur = this->_data._finish._first + _num_elts % data_type::_S_buffer_size();
    }
    void _M_create_nodes(map_pointer _start, map_pointer _finish) {
        map_pointer _cur;
        for (_cur = _start; _cur != _finish; ++_cur) {
            *_cur = this->_M_allocate_node();
        }
    }
    void _M_destory_nodes(map_pointer _start, map_pointer _finish) {
        map_pointer _cur;
        for (_cur = _start; _cur != _finish; ++_cur) {
            this->_M_deallocate_node(*_cur);
        }
    }

    data_type _data;
private:
    data_type _M_move_data() {

    }
};

};

#endif