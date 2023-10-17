#ifndef _ASP_UNION_FIND_H_
#define _ASP_UNION_FIND_H_

#include "uf_table.hpp"
#include "log_utils.hpp"

namespace asp {

template <typename _Tp, typename _Hash = std::hash<_Tp>, typename _Alloc = std::allocator<_Tp>> class uf_set;
template <typename _Key, typename _Tp, typename _Hash = std::hash<_Tp>, typename _Alloc = std::allocator<_Tp>> class uf_map;

/**
 * @brief union_find_set
 * @details 
*/

template <typename _Tp, typename _Hash, typename _Alloc> class uf_set {
public:
    typedef uf_set<_Tp, _Hash, _Alloc> self;
    typedef uf_table<_Tp, _Tp, _select_self, _select_self, _Hash, _Alloc> uf_t;
    typedef typename uf_t::key_type key_type;
    typedef typename uf_t::value_type value_type;
    typedef typename uf_t::mapped_type mapped_type;
private:
    uf_t _ut;

public:
    uf_set() = default;
    uf_set(const self& _rhs) : _ut(_rhs._ut) {}
    self& operator=(const self& _rhs) {
        if (&_rhs == this) return *this;
        _ut = _rhs._ut; return *this;
    }
    virtual ~uf_set() = default;

    size_type size() const { return _ut.size(); }
    size_type classification() const { return _ut.classification(); }
    bool add(const value_type& _e) { return _ut.add(_e); }
    bool add(const key_type& _t, const value_type& _e) { return _ut.add(_t, _e); }
    bool del(const key_type& _k) { return _ut.del(_k); }
    bool merge(const key_type& _x, const key_type& _y) { return _ut.merge(_x, _y); }
    bool elect(const key_type& _k) { return _ut.elect(_k); }
    void clear() { _ut.clear(); }
    bool empty() const { return _ut.empty(); }
    bool existed(const key_type& _k) const { return _ut.existed(_k); }
    bool sibling(const key_type& _x, const key_type& _y) { return _ut.sibling(_x, _y); }

    int check() const { return _ut.check(); }
    void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);
    template <typename _T, typename _H, typename _A> friend std::ostream& operator<<(std::ostream& _os, const uf_set<_T, _H, _A>& _x);
private:
    enum operator_id {
        __ADD__, __MOUNT__, __DEL__,
        __MERGE__, __ELECT__, __EXISTED__, __SIBLING__,
        __CLEAR__, __SIZE__, __CL_SIZE__,
        __PRINT__,
        __NONE__,
    };
    const std::unordered_map<std::string, operator_id> _operator_map = {
        {"add", __ADD__}, {"mount", __MOUNT__}, {"del", __DEL__},
        {"merge", __MERGE__}, {"elect", __ELECT__},
        {"existed", __EXISTED__}, {"sibling", __SIBLING__},
        {"clear", __CLEAR__}, {"size", __SIZE__}, {"cl_size", __CL_SIZE__},
        {"print", __PRINT__}
    };
    operator_id _M_get_operator_id(const std::string& _op) {
        auto _it = _operator_map.find(_op);
        return _it != _operator_map.cend() ? _it->second : __NONE__;
    }
};
template <typename _Tp, typename _Hash, typename _Alloc> auto
uf_set<_Tp, _Hash, _Alloc>::demo(std::istream& _is, std::ostream& _os) -> void {
    _os << '[' << typeid(asp::decay_t<self>).name() << ']' << std::endl;
    _is.sync_with_stdio(false);
    std::string _op;
    key_type _k;
    key_type _k2;
    mapped_type _m;
    value_type _v;
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
template <typename _T, typename _H, typename _A> auto
operator<<(std::ostream& _os, const uf_set<_T, _H, _A>& _x) -> std::ostream& {
    _os << '[';
    for (auto _i = _x._ut._sth.cbegin(); _i != _x._ut._sth.cend(); ++_i) {
        _os << *(_i->second);
    }
    return _os << ']';
}

// template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> class uf_set {
//     uf_table<_Key, std::pair<_Key, _Tp>, _select_self, _select_self, _Hash, _Alloc> _ut;
// public:
//     bool sibling(const key_type& _x, const key_type& _y);
//     size_type size() const { return _m_content.size(); }
//     size_type sets_count() const { return _m_branch_count; }
//     bool insert(const key_type& _e);
//     bool insert(const key_type& _s, const key_type& _e);
//     size_type erase(const key_type& _e);
//     void merge(const key_type& _x, const key_type& _y);
// };


};

#endif // _ASP_UNION_FIND_H_