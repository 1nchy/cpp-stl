#ifndef _ASP_UNORDERED_MULTISET_HPP_
#define _ASP_UNORDERED_MULTISET_HPP_

#include <functional>

#include "basic_param.hpp"
#include "hash_table.hpp"

namespace asp {

template <typename _Tp,
 typename _Hash = std::hash<_Tp>,
 typename _Alloc = std::allocator<_Tp>
> class unordered_multiset;

template <typename _Tp, typename _Hash, typename _Alloc>
class unordered_multiset {
    typedef unordered_multiset<_Tp, _Hash, _Alloc> self;
    typedef hash_table<_Tp, _Tp, _select_self, false, _select_self, _Hash, _Alloc> umset_ht;
    umset_ht _h;
public:
    typedef typename umset_ht::key_type key_type;
    typedef typename umset_ht::value_type value_type;
    typedef typename umset_ht::mapped_type mapped_type;
    typedef typename umset_ht::hasher hasher;
    typedef typename umset_ht::iterator iterator;
    typedef typename umset_ht::const_iterator const_iterator;
    typedef typename umset_ht::ireturn_type ireturn_type;
    typedef typename umset_ht::insert_status insert_status;
    typedef typename umset_ht::ext_iterator ext_iterator;
    typedef typename umset_ht::ext_key ext_key;
    typedef typename umset_ht::ext_value ext_value;

/// (de)constructor
    unordered_multiset() = default;
    unordered_multiset(const self& _x) : _h(_x._h) {}
    virtual ~unordered_multiset() = default;
    
/// implement
    size_type size() const { return _h.size(); }
    bool empty() const { return _h.empty(); }
    iterator begin() { return _h.begin(); }
    iterator end() { return _h.end(); }
    const_iterator cbegin() const { return _h.cbegin(); }
    const_iterator cend() const { return _h.cend(); }
    ireturn_type insert(const value_type& _v) { return _h.insert(_v); }
    size_type erase(const key_type& _k) { return _h.erase(_k); }
    size_type count(const key_type& _k) const { return _h.count(_k); }
    void clear() { _h.clear(); }
    iterator find(const key_type& _k) { return _h.find(_k); }
    const_iterator find(const key_type& _k) const { return _h.find(_k); }
#ifdef _CONTAINER_CHECK_
    int check() const { return _h.check(); }
#endif // _CONTAINER_CHECK_
    void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);

/// output
    template <typename _T, typename _H, typename _A>
     friend std::ostream& operator<<(std::ostream& os, const unordered_multiset<_T, _H, _A>& _um);
    // friend std::ostream& operator<<(std::ostream& os, const const_iterator& _i);
private:
    enum operator_id {
        __ADD__, __DEL__,
        __COUNT__,
        __CLEAR__, __SIZE__,
        __PRINT__,
        __NONE__,
    };
    const std::unordered_map<std::string, operator_id> _operator_map = {
        {"add", __ADD__}, {"del", __DEL__},
        {"count", __COUNT__},
        {"clear", __CLEAR__}, {"size", __SIZE__},
        {"print", __PRINT__}
    };
    operator_id _M_get_operator_id(const std::string& _op) {
        auto _it = _operator_map.find(_op);
        return _it != _operator_map.cend() ? _it->second : __NONE__;
    }
};

template <typename _Tp, typename _Hash, typename _Alloc> auto
operator<<(std::ostream& os, const unordered_multiset<_Tp, _Hash, _Alloc>& _um)
-> std::ostream& {
    os << _um._h;
    return os;
};

template <typename _Tp, typename _Hash, typename _Alloc> auto
unordered_multiset<_Tp, _Hash, _Alloc>::demo(std::istream& _is, std::ostream& _os) -> void {
    _os << '[' << typeid(asp::decay_t<self>).name() << ']' << std::endl;
    _is.sync_with_stdio(false);
    std::string _op;
    // key_type _k;
    mapped_type _m;
    // value_type _v;
    size_type _n;
    while (!_is.eof()) {
        _is >> _op;
        if (__details__::_M_end_of_file(_is)) break;
        operator_id _id = this->_M_get_operator_id(_op);
        switch (_id) {
        case __ADD__: {
            _is >> _v;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->add(_v);
            if (_r) _os << "add(" << _v << ")" << std::endl;
        }; break;
        case __MOUNT__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            _is >> _v;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->add(_k, _v);
            if (_r) _os << "mount(" << _k << ", " << _v << ")" << std::endl;
        }; break;
        case __DEL__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->del(_k);
            if (_r) _os << "del(" << _k << ")" << std::endl;
        }; break;
        case __MERGE__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            _is >> _k2;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->merge(_k, _k2);
            if (_r) _os << "merge(" << _k << ", " << _k2 << ")" << std::endl;
        }; break;
        case __ELECT__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->elect(_k);
            if (_r) _os << "elect(" << _k << ")" << std::endl;
        }; break;
        case __EXISTED__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->existed(_k);
            _os << "existed(" << _k << ") = " << _r << std::endl;
        }; break;
        case __SIBLING__: {
            _is >> _k;
            if (__details__::_M_end_of_file(_is)) break;
            _is >> _k2;
            if (__details__::_M_end_of_file(_is)) break;
            const auto _r = this->sibling(_k, _k2);
            _os << "sibling(" << _k << ", " << _k2 << ") = " << _r << std::endl;
        }; break;
        case __CLEAR__: {
            this->clear();
        }; break;
        case __SIZE__: {
            _os << ": " << this->size() << std::endl;
        }; break;
        case __CL_SIZE__: {
            _os << ": " << this->classification() << std::endl;
        }; break;
        case __PRINT__:{
            _os << *this << std::endl;
        }; break;
        case __NONE__:{}; break;
        }
        _op.clear();
        __details__::_M_reset_cin(_is);
        if (auto _ret = this->check()) {
            ASP_ERR("error(%d) in container.\n", _ret);
            break;
        }
        _os << std::flush;
        // _os << *this << std::endl;
    }
    __details__::_M_reset_cin(_is);
};

};

#endif // _ASP_UNORDERED_MULTISET_HPP_
