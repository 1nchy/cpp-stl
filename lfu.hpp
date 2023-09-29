#ifndef _ASP_LFU_HPP_
#define _ASP_LFU_HPP_

#include "hash_table.hpp"
#include "list.hpp"
#include <unordered_map>

namespace asp {

/**
 * @brief least frequently used structure
 * @details
 *    key(freq) in list, assume that key == value, structure like below:
 *    --------------------------------------------------
 *       _kt   4     5     8     2     6     1     9
 *     _list [ 4(4), 5(4), 8(2), 2(2), 6(2), 1(1), 9(1) ] end
 *       _ft   4           2                 1
 *    --------------------------------------------------
 *    noticed that, if we need to insert {k, f}, f = 1 or _ft[f-1] existed.
 *    former one means putting a k-v, latter one means getting a k, and increase frequence of k.
 * 
 *    list node structure: [key, val, freq]
 *      for %_kt            key  (  val  )
 *      for %_ft            (  val  )  key
*/
template <typename _Key, typename _Tp,
 typename _Hash = std::hash<_Key>,
 typename _Alloc = std::allocator<std::tuple<const _Key, _Tp, size_type>>
> class lfu;

namespace {
struct _select_key_from_lfu_node {
    template <typename _Tp> auto operator()(_Tp&& _x) const {
        return std::get<0>(*_x);
    };
};
struct _select_freq_from_lfu_node {
    template <typename _Tp> auto operator()(_Tp&& _x) const {
        return std::get<2>(*_x);
    };
};
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc>
class lfu {
    typedef lfu<_Key, _Tp, _Hash, _Alloc> self;
    typedef _Key key_type;
    typedef _Tp value_type;
    typedef list<std::tuple<const _Key, _Tp, size_type>, _Alloc> list_t;
    typedef typename list_t::node_type node_type;
    typedef typename list_t::iterator iterator;
    typedef hash_table<_Key, iterator, _select_key_from_lfu_node, true, _select_self_ref, _Hash, _Alloc> key_table_t;
    typedef hash_table<_Key, iterator, _select_freq_from_lfu_node, true, _select_self_ref, _Hash, _Alloc> freq_table_t;

    list_t _l;
    key_table_t _kt;  // key table
    freq_table_t _ft;  // frequence table
    size_type _capacity;
    _select_key_from_lfu_node _select_key;
    _select_freq_from_lfu_node _select_freq;

public:
    typedef typename list_t::const_iterator const_iterator;
/// (de)constructor
    lfu(size_type _capacity) : _capacity(_capacity) {}
    lfu(const self& _rhs) : _l(_rhs._l), _kt(_rhs._kt), _ft(_rhs._ft), _capacity(_rhs._capacity) {}
    self& operator=(const self& _rhs);
    virtual ~lfu() = default;

    size_type size() const { return _l.size(); }
    size_type capacity() const { return _capacity; }
    bool empty() const { return _l.empty(); }
    void clear() { _ft.clear(); _kt.clear(); _l.clear(); }
    void resize(size_type _new_capacity);
    const_iterator get(const key_type& _k);
    void put(const key_type& _k, const value_type& _v);

    const_iterator cbegin() const { return _l.cbegin(); }
    const_iterator cend() const { return _l.cend(); }

    template <typename _K, typename _T, typename _H, typename _A> friend std::ostream& operator<<(std::ostream& _os, const lfu<_K, _T, _H, _A>& _x);

    int check() const;
    void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);

private:
    void increase_freq(const key_type& _k);
    void eliminate(size_type _step = 0);
    void eliminate_last();
    inline bool existed(const key_type& _k) const { return _kt.count(_k) != 0; }
    inline size_type frequence(const key_type& _k) const { return existed(_k) ? _select_freq(*(_kt.find(_k))) : 0; }

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
lfu<_Key, _Tp, _Hash, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs == this) return *this;
    _l.operator=(_rhs._l);
    _kt.operator=(_rhs._kt);
    _ft.operator=(_rhs._ft);
    _capacity = _rhs._capacity;
};
template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::resize(size_type _new_capacity) -> void {
    _capacity = _new_capacity;
    eliminate();
};
template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::get(const key_type& _k) -> const_iterator {
    if (!existed(_k)) return _l.cend();
    increase_freq(_k);
    const_iterator _i(*(_kt.find(_k)));
    return _i;
};
template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::put(const key_type& _k, const value_type& _v) -> void {
    if (!existed(_k)) {
        if (size() == _capacity) eliminate(1);
        if (_ft.count(1) == 0) { // node whose freq = 1, not existed
            iterator _inserted_i = _l.insert(_l.end(), {_k, _v, 1});
            _kt.update(_inserted_i);
            _ft.update(_inserted_i);
        }
        else {
            iterator _ii = *(_ft.find(1));
            iterator _inserted_i = _l.insert(_ii, {_k, _v, 1});
            _kt.update(_inserted_i);
            _ft.update(_inserted_i);
        }
    }
    else {
        iterator _i = *(_kt.find(_k));
        std::get<1>(*_i) = _v;
        increase_freq(_k);
    }
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::increase_freq(const key_type& _k) -> void {
    if (!existed(_k)) return;
    const size_type _freq = frequence(_k);
    iterator _i = *(_kt.find(_k));
    iterator _insert_i = *(_ft.count(_freq + 1) == 0 ? _ft.find(_freq) : _ft.find(_freq + 1));
    if (*(_ft.find(_freq)) == _i) {
        ++_i;
        if (_i != _l.end() && _select_freq(_i) == _freq) {
            *(_ft.find(_freq)) = _i;
        }
        else {
            _ft.erase(_freq);
        }
    }
    iterator _inserted_i = _l.insert(_insert_i, *(*_kt.find(_k)));
    std::get<2>(*(_inserted_i._const_cast())) = _freq + 1;
    _l.erase(*(_kt.find(_k)));
    _kt.update(_inserted_i);
    _ft.update(_inserted_i);
};
template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::eliminate(size_type _step) -> void {
    if (_step == 0) {
        while (_l.size() > _capacity) {
            eliminate_last();
        }
    }
    else {
        while (_step > 0 && !_l.empty()) {
            eliminate_last();
            if (_step > 0) --_step;
        }
    }
};
template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::eliminate_last() -> void {
    if (!_l.empty()) {
        auto _back_i = _l.end(); --_back_i;
        if (*(_ft.find(_select_freq(_back_i))) == _back_i) {
            _ft.erase(_select_freq(_back_i));
        }
        _kt.erase(_select_key(_back_i));
        _l.pop_back();
    }
};

template <typename _K, typename _T, typename _H, typename _A> auto
operator<<(std::ostream& _os, const lfu<_K, _T, _H, _A>& _x)-> std::ostream& {
    return _os << _x._l;
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::check() const -> int {
    return 0;
};

template <typename _Key, typename _Tp, typename _Hash, typename _Alloc> auto
lfu<_Key, _Tp, _Hash, _Alloc>::demo(std::istream& _is, std::ostream& _os) -> void {
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
                _os << std::get<1>(*_r);
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

#endif // _ASP_LFU_HPP_