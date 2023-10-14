#ifndef _ASP_LRU_HPP_
#define _ASP_LRU_HPP_

#include "lru_table.hpp"

namespace asp {

template <typename _Key, typename _Tp,
 typename _Hash = std::hash<_Key>,
 typename _Alloc = std::allocator<std::pair<const _Key, _Tp>>
> class lru_map;
template <typename _Tp,
 typename _Hash = std::hash<_Tp>,
 typename _Alloc = std::allocator<_Tp>
> class lru_set;

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc>
class lru_map {
    typedef lru_map<_Key, _Tp, _Hash, _Alloc> self;
    typedef lru_table<_Key, std::pair<const _Key, _Tp>, _select_0x, _select_1x, _Hash, _Alloc> lru_t;
    typedef typename lru_t::key_type key_type;
    typedef typename lru_t::value_type value_type;
    typedef typename lru_t::mapped_type mapped_type;
    // typedef typename lru_t::node_type node_type;
    typedef typename lru_t::iterator iterator;
    typedef typename lru_t::const_iterator const_iterator;

    lru_t _l;

public:

/// (de)constructor
    lru_map(size_type _capacity) : _l(_capacity) {}
    lru_map(const self& _rhs) : _l(_rhs._l) {}
    self& operator=(const self& _rhs) {
        if (&_rhs == this) return *this;
        _l.operator=(_rhs._l);
        return *this;
    }
    virtual ~lru_map() = default;

    size_type size() const { return _l.size(); }
    size_type capacity() const { return _l.capacity(); }
    bool empty() const { return _l.empty(); }
    void clear() { _l.clear(); }
    void resize(size_type _new_capacity) { _l.resize(_new_capacity); }
    const_iterator get(const key_type& _k) { return _l.get(_k); }
    void put(const value_type& _v) { _l.put(_v); }

    const_iterator cbegin() const { return _l.cbegin(); }
    const_iterator cend() const { return _l.cend(); }

    template <typename _K, typename _T, typename _H, typename _A> friend std::ostream& operator<<(std::ostream& _os, const lru_map<_K, _T, _H, _A>& _x);

    int check() const;
    void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);

private:
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

template <typename _Tp, typename _Hash, typename _Alloc>
class lru_set {
    typedef lru_set<_Tp, _Hash, _Alloc> self;
    typedef lru_table<_Tp, _Tp, _select_self, _select_self, _Hash, _Alloc> lru_t;
    typedef typename lru_t::key_type key_type;
    typedef typename lru_t::value_type value_type;
    typedef typename lru_t::mapped_type mapped_type;
    // typedef typename lru_t::node_type node_type;
    typedef typename lru_t::iterator iterator;
    typedef typename lru_t::const_iterator const_iterator;

    lru_t _l;
public:
/// (de)constructor
    lru_set(size_type _capacity) : _l(_capacity) {}
    lru_set(const self& _rhs) : _l(_rhs._l) {}
    self& operator=(const self& _rhs) {
        if (&_rhs == this) return *this;
        _l.operator=(_rhs._l);
        return *this;
    }
    virtual ~lru_set() = default;

    size_type size() const { return _l.size(); }
    size_type capacity() const { return _l.capacity(); }
    bool empty() const { return _l.empty(); }
    void clear() { _l.clear(); }
    void resize(size_type _new_capacity) { _l.resize(_new_capacity); }
    const_iterator get(const key_type& _k) { return _l.get(_k); }
    void put(const value_type& _v) { _l.put(_v); }

    const_iterator cbegin() const { return _l.cbegin(); }
    const_iterator cend() const { return _l.cend(); }

    template <typename _T, typename _H, typename _A> friend std::ostream& operator<<(std::ostream& _os, const lru_set<_T, _H, _A>& _x);

    int check() const;
    void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);

private:
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
lru_map<_Key, _Tp, _Hash, _Alloc>::check() const -> int {
    return 0;
};
template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lru_map<_Key, _Tp, _Hash, _Alloc>::demo(std::istream& _is, std::ostream& _os) -> void {
    _os << '[' << typeid(asp::decay_t<self>).name() << ']' << std::endl;
    _is.sync_with_stdio(false);
    std::string _op;
    key_type _k;
    mapped_type _m;
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
            _is >> _m;
            if (__details__::_M_end_of_file(_is)) break;
            this->put({_k, _m});
            _os << "put(" << _k << ", " << _m << ")" << std::endl;
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
template <typename _K, typename _T, typename _H, typename _A> auto
operator<<(std::ostream& _os, const lru_map<_K, _T, _H, _A>& _x)
-> std::ostream& {
    _os << _x._l;
    return _os;
};


template <typename _Tp, typename _Hash, typename _Alloc> auto
lru_set<_Tp, _Hash, _Alloc>::check() const -> int {
    return 0;
};
template <typename _Tp, typename _Hash, typename _Alloc> auto
lru_set<_Tp, _Hash, _Alloc>::demo(std::istream& _is, std::ostream& _os) -> void {
    _os << '[' << typeid(asp::decay_t<self>).name() << ']' << std::endl;
    _is.sync_with_stdio(false);
    std::string _op;
    value_type _v;
    size_type _n;
    while (!_is.eof()) {
        _is >> _op;
        if (__details__::_M_end_of_file(_is)) break;
        operator_id _id = this->_M_get_operator_id(_op);
        switch (_id) {
        case __GET__: {
            _is >> _v;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->get(_v);
            _os << "get(" << _v << ") = ";
            if (_r == this->cend()) {
                _os << "none";
            }
            else {
                _os << *_r;
            }
            _os << std::endl;
        }; break;
        case __PUT__: {
            _is >> _v;
            if (__details__::_M_end_of_file(_is)) break;
            this->put(_v);
            _os << "put(" << _v << ")" << std::endl;
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
template <typename _T, typename _H, typename _A> auto
operator<<(std::ostream& _os, const lru_set<_T, _H, _A>& _x)
-> std::ostream& {
    _os << _x._l;
    return _os;
};
};

#endif // _ASP_LRU_HPP_