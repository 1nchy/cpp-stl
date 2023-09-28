#ifndef _ASP_LRU_HPP_
#define _ASP_LRU_HPP_

#include "hash_table.hpp"
#include "list.hpp"

namespace asp {

template <typename _Key, typename _Tp,
 typename _Hash = std::hash<_Key>,
 typename _Alloc = std::allocator<std::pair<const _Key, _Tp>>
> struct lru;

namespace __details__ {
struct _select_key_from_list_node {
    // _Tp == list_t::iterator
    template <typename _Tp> auto operator()(_Tp&& _x) const {
        return _x->first;
    }
};
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc>
class lru {
    typedef lru<_Key, _Tp, _Hash, _Alloc> self;
    typedef _Key key_type;
    typedef _Tp value_type;
    typedef list<std::pair<const _Key, _Tp>, _Alloc> list_t;
    typedef typename list_t::node_type node_type;
    typedef typename list_t::iterator iterator;
    typedef hash_table<_Key, iterator, __details__::_select_key_from_list_node, true, _Hash, _Alloc> hash_table_t;

    list_t _l;
    hash_table_t _h;
    size_type _capacity;

public:
    typedef typename list_t::const_iterator const_iterator;

/// (de)constructor
    lru(size_type _capacity) : _capacity(_capacity) {}
    lru(const self& _rhs) : _l(_rhs._l), _h(_rhs._h), _capacity(_rhs._capacity) {}
    self& operator=(const self& _rhs);
    virtual ~lru() = default;

    size_type size() const { return _l.size(); }
    size_type capacity() const { return _capacity; }
    bool empty() const { return _l.empty(); }
    void clear() { _h.clear(); _l.clear(); }
    void resize(size_type _new_capacity);
    const_iterator get(const key_type& _k);
    void put(const key_type& _k, const value_type& _v);

    const_iterator cbegin() const { return _l.cbegin(); }
    const_iterator cend() const { return _l.cend(); }

    template <typename _K, typename _T, typename _H, typename _A> friend std::ostream& operator<<(std::ostream& _os, const lru<_K, _T, _H, _A>& _x);

    int check() const;
    void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);

private:
    void eliminate();

    enum operator_id {
        __GET__, __PUT__,
        __CLEAR__, __SIZE__, __RESIZE__,
        __PRINT__,
        __NONE__,
    };
    const std::unordered_map<std::string, operator_id> _operator_map = {
        {"get", __GET__}, {"put", __PUT__},
        {"clear", __CLEAR__}, {"size", __SIZE__},
        {"resize", __RESIZE__}, {"print", __PRINT__}
    };
    operator_id _M_get_operator_id(const std::string& _op) {
        auto _it = _operator_map.find(_op);
        return _it != _operator_map.cend() ? _it->second : __NONE__;
    }
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru<_Key, _Tp, _Hash, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs == this) return *this;
    _l.operator=(_rhs._l);
    _h.operator=(_rhs._h);
    _capacity = _rhs._capacity;
}

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru<_Key, _Tp, _Hash, _Alloc>::resize(size_type _new_capacity) -> void {
    _capacity = _new_capacity;
    eliminate();
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru<_Key, _Tp, _Hash, _Alloc>::get(const key_type& _k) -> const_iterator {
    if (_h.count(_k) == 0) return _l.cend();
    const_iterator _i = *(_h.find(_k));
    _l.move_2_front(_i);
    return _i;
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru<_Key, _Tp, _Hash, _Alloc>::put(const key_type& _k, const value_type& _v) -> void {
    if (_h.count(_k) == 0) {
        _l.push_front(std::make_pair(_k, _v));
        _h.insert(_l.begin());
        eliminate();
    }
    else {
        iterator _i = *(_h.find(_k));
        (*_i).second = _v;
        _l.move_2_front(_i);
    }
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru<_Key, _Tp, _Hash, _Alloc>::eliminate() -> void {
    while (_l.size() > _capacity) {
        _h.erase(_l.back().first);
        _l.pop_back();
    }
};

template <typename _K, typename _T, typename _H, typename _A> auto
operator<<(std::ostream& _os, const lru<_K, _T, _H, _A>& _x)-> std::ostream& {
    return _os << _x._l;
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru<_Key, _Tp, _Hash, _Alloc>::check() const -> int {
    return 0;
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru<_Key, _Tp, _Hash, _Alloc>::demo(std::istream& _is, std::ostream& _os) -> void {
    _os << '[' << typeid(asp::decay_t<self>).name() << ']' << std::endl;
    _is.sync_with_stdio(false);
    std::string _op;
    key_type _k;
    value_type _v;
    size_type _n;
    while (!_is.eof()) {
        _is >> _op;
        if (__details__::_M_end_of_file(_is)) break;
        operator_id _id = this->_M_get_operator_id(_op);
        switch (_id) {
        case __GET__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->get(_k);
            _os << "get(" << _k << ") = ";
            if (_r == this->cend()) {
                _os << "none";
            }
            else {
                _os << *_r;
            }
            _os << std::endl;
        }; break;
        case __PUT__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            _is >> _v;
            if (__details__::_M_end_of_file(_is)) break;
            this->put(_k, _v);
            _os << "put(" << _k << ", " << _v << ")" << std::endl;
        }; break;
        case __CLEAR__: {
            this->clear();
        }; break;
        case __SIZE__: {
            _os << ": " << this->size() << std::endl;
        }; break;
        case __RESIZE__: {
            _is >> _n;
            if (__details__::_M_end_of_file(_is)) break;
            this->resize(_n);
        }; break;
        case __PRINT__:{
            _os << *this << std::endl;
        }; break;
        case __NONE__:{}; break;
        }
        _op.clear();
        __details__::_M_reset_cin(_is);
        _os << std::flush;
        // _os << *this << std::endl;
    }
    __details__::_M_reset_cin(_is);
};


};

#endif // _ASP_LRU_HPP_