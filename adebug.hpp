#ifndef _ASP_DEBUG_HPP_
#define _ASP_DEBUG_HPP_

#include <ostream>
#include <iostream>
#include <sstream>
#include <functional>
#include <map>
#include <unordered_map>

#include <ctime>

#include "basic_param.hpp"
#include "log_utils.hpp"
#include "type_traits.hpp"
#include "basic_io.hpp"
#include "random.hpp"

/**
 * automatic test of container.
 * register the function, and invoke them randomly to test the container.
 * 
 * the container in stl, could be divided into 2 part:
 * sequence container and associative container.
 * 
 * for all container, it must implement @size, @clear.
 * for sequence container, it must implement @push & @pop, @insert, @erase.
 * for associative container, it must implement @insert, @erase, @count, @find.
*/

namespace asp {

template <typename _Container> struct debug_base;
template <typename _SequenceContainer> struct debug_seq_container;
template <typename _AssociativeContainer> struct debug_asso_container;

static void _S_pause() {}

template <typename _Container> struct debug_base {
    typedef debug_base<_Container> self;
    typedef _Container container_type;
    typedef typename _Container::value_type value_type;
    typedef typename _Container::iterator iterator;
    typedef typename _Container::const_iterator const_iterator;

    typedef int (*out_check_fptr)(const _Container&);
    typedef int (_Container::*check_fptr)() const;
    typedef void (_Container::*clear_fptr)();

    typedef size_type (_Container::*size_fptr)() const;

    out_check_fptr _out_check = nullptr;
    check_fptr _check = nullptr;
    clear_fptr _clear = &container_type::clear;
    size_fptr _size = &container_type::size;

    container_type _container;

    size_type _circle_count = 1000;

    debug_base() = default;
    // debug_base(const self& _da) : _out_check(_da._out_check), _clear(_da._clear), _size(_da._size), _container(_da._container) {}
    virtual ~debug_base() = default;

    void demo();
    void auto_test(const std::string& _str = "");
    virtual void init_stream(std::stringstream& _is, size_type _n) = 0;
    virtual void demo_from_istream(std::istream& _is, bool _log, bool _print_container) = 0;


    void _M_print_container() const { std::cout << _container << std::endl;}

    int _M_reg_check() const;
    void _M_reg_clear(bool _log = false);
    void _M_reg_size(bool _log = false) const;

    // std::string _M_string_from_iterator(iterator _i) const;
    std::string _M_string_from_iterator(const_iterator _i) const;

    enum operator_id {
        __PUSH_BACK__, __POP_BACK__,
        __PUSH_FRONT__, __POP_FRONT__,
        __INSERT__, __ERASE__,
        __ADD__, __SET__, __DELETE__,
        __CLEAR__, __COUNT__, __SIZE__, __FIND__,
        __PRINT__,
        __QUIT__,
        __NONE__,
        __PAUSE__, // used to debug
    };

    const std::unordered_map<std::string, operator_id> _operator_map = {
        {"push_back", __PUSH_BACK__}, {"pop_back", __POP_BACK__},
        {"push", __PUSH_BACK__}, {"pop", __POP_BACK__},
        {"pushb", __PUSH_BACK__}, {"popb", __POP_BACK__},
        {"push_front", __PUSH_FRONT__}, {"pop_front", __POP_FRONT__},
        {"pushf", __PUSH_FRONT__}, {"popf", __POP_FRONT__},
        {"insert", __INSERT__}, {"erase", __ERASE__},
        {"add", __ADD__}, {"a", __ADD__}, {"set", __SET__}, {"s", __SET__}, {"del", __DELETE__}, {"d", __DELETE__},
        {"clear", __CLEAR__}, {"count", __COUNT__}, {"size", __SIZE__}, {"find", __FIND__}, {"f", __FIND__},
        {"i", __INSERT__}, {"e", __ERASE__},
        {"l", __CLEAR__}, {"c", __COUNT__},
        {"quit", __QUIT__}, {"q", __QUIT__},
        {"print", __PRINT__}, {"p", __PRINT__},
        {"pause", __PAUSE__}
    };
    
    operator_id _M_get_operator_id(const std::string& _op) {
        auto _it = _operator_map.find(_op);
        return _it != _operator_map.cend() ? _it->second : __NONE__;
    }

protected:
    _HAS_OPERATOR(<<, out);
};

template <typename _SeqContainer> struct debug_seq_container : public debug_base<_SeqContainer> {
    typedef debug_base<_SeqContainer> base;
    typedef debug_seq_container<_SeqContainer> self;
    typedef typename base::container_type container_type;
    typedef typename base::out_check_fptr out_check_fptr;
    typedef typename base::check_fptr check_fptr;
    typedef typename base::value_type value_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef typename base::operator_id operator_id;
/// container function typedef
    typedef void (container_type::*push_fptr)(const value_type&);
    typedef void (container_type::*pop_fptr)();
    typedef iterator (container_type::*insert_fptr)(const_iterator, const value_type&);
    typedef iterator (container_type::*earse_fptr)(const_iterator);
    typedef typename base::size_fptr size_fptr;
/// (de)constructor
    debug_seq_container() = default;
    // debug_seq_container(const self& _da) : base(_da)
    // , _push_back(_da._push_back), _pop_back(_da._pop_back), _push_front(_da._push_front), _pop_front(_da._pop_front)
    // , _insert(_da._insert), _erase(_da._erase) {}

/// member
    push_fptr _push_back = nullptr;
    pop_fptr _pop_back = nullptr;
    push_fptr _push_front = nullptr;
    pop_fptr _pop_front = nullptr;
    insert_fptr _insert = nullptr;
    earse_fptr _erase = nullptr;

/// register function
    void _M_reg_push_back(const value_type& _v, bool _log = false);
    void _M_reg_pop_back(bool _log = false);
    void _M_reg_push_front(const value_type& _v, bool _log = false);
    void _M_reg_pop_front(bool _log = false);
    void _M_reg_insert(const_iterator _i, const value_type& _v, bool _log = false);
    void _M_reg_erase(const_iterator _i, bool _log = false);

/// demo function
    void demo_from_istream(std::istream& _is, bool _log, bool _print_container) override;

    void init_stream(std::stringstream& _is, size_type _n) override;

protected:
/// helper
    size_type _M_get_positive_offset(difference_type _i, bool _del = false) const;
};

template <typename _AssoContainer> struct debug_asso_container : public debug_base<_AssoContainer> {
    typedef debug_asso_container<_AssoContainer> self;
    typedef debug_base<_AssoContainer> base;
    typedef typename base::container_type container_type;
    typedef typename base::out_check_fptr out_check_fptr;
    typedef typename base::check_fptr check_fptr;
    typedef typename base::value_type value_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef typename container_type::key_type key_type;
    typedef typename container_type::mapped_type mapped_type;
    typedef typename container_type::ireturn_type ireturn_type;
    typedef typename base::operator_id operator_id;
/// container function typedef
    typedef ireturn_type (container_type::*insert_fptr)(const value_type&);
    // typedef ireturn_type (container_type::*set_fptr)(const key_type&, const mapped_type&);
    typedef size_type (container_type::*earse_fptr)(const key_type&);
    // typedef iterator (container_type::*find_fptr)(const key_type&);
    typedef const_iterator (container_type::*find_fptr)(const key_type&) const;
    typedef size_type (container_type::*count_fptr)(const key_type&) const;
    typedef typename base::size_fptr size_fptr;
/// (de)constructor
    debug_asso_container() : base() {}
    // debug_asso_container(const self& d) : base(d) {} //, _insert(d._insert), _set(d._set), _erase(d._erase), _find(d._find), _cfind(d._cfind), _count(d._count) {}
/// member
    insert_fptr _insert = nullptr;
    // set_fptr _set = nullptr;
    earse_fptr _erase = nullptr;
    find_fptr _find = nullptr;
    count_fptr _count = nullptr;

/// register function
    void _M_reg_insert(const value_type& _v, bool _log = false);
    // void _M_reg_set(const key_type& _k, const mapped_type& _v, bool _log = false);
    void _M_reg_erase(const key_type& _k, bool _log = false);
    void _M_reg_count(const key_type& _k, bool _log = false) const;
    void _M_reg_find(const key_type& _k, bool _log = false) const;

/// demo function
    void demo_from_istream(std::istream& _is, bool _log, bool _print_container) override;

    void init_stream(std::stringstream& _is, size_type _n) override;

private:
    bool _b_kv_self = asp::is_same<key_type, value_type>::value;
    value_type _M_get_value_from(std::istream&) const;
};

static bool _M_end_of_file(std::istream& _is = std::cin) {
    return _is.eof() || _is.fail();
};
static void _M_reset_cin(std::istream& _is = std::cin) {
    _is.clear(); // _is.sync();
};

template <typename _C> void debug_base<_C>::demo() {
    std::cout << '[' << typeid(asp::decay_t<_C>).name() << "]:" << std::endl;
    this->demo_from_istream(std::cin, true, true);
};
template <typename _C> void debug_base<_C>::auto_test(const std::string& _str) {
    std::stringstream _ss;
    if (!_str.empty()) {
        _ss.str(_str);
    }
    else {
        this->init_stream(_ss, _circle_count);
        // const std::string _order_str = _ss.str();
        // ASP_LOG("order: %s.\n", _order_str.c_str());
        // _ss.clear();
        // _ss.str(_order_str);
    }
    this->demo_from_istream(_ss, true, true);
};

template <typename _SC> void debug_seq_container<_SC>::demo_from_istream(std::istream& _is, bool _log, bool _print_container) {
    size_type _i; // iterator_index
    difference_type _di; // input_index
    value_type _v;
    std::string _op;
    while (!_is.eof()) {
        _is >> _op;
        if (_M_end_of_file(_is)) { break; }
        operator_id _id = this->_M_get_operator_id(_op);
        switch (_id) {
        case base::__PUSH_BACK__: {
            _is >> _v;
            if (_M_end_of_file(_is)) { break; }
            this->_M_reg_push_back(_v, _log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__POP_BACK__: {
            this->_M_reg_pop_back(_log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__PUSH_FRONT__: {
            _is >> _v;
            if (_M_end_of_file(_is)) { break; }
            this->_M_reg_push_front(_v, _log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__POP_FRONT__: {
            this->_M_reg_pop_front(_log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__INSERT__: {
            _is >> _di;
            if (_M_end_of_file(_is)) { break; }
            _is >> _v;
            if (_M_end_of_file(_is)) { break; }
            _i = _M_get_positive_offset(_di, false);
            const_iterator _p = this->_container.cbegin() + _i;
            this->_M_reg_insert(_p, _v, _log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__ERASE__: {
            _is >> _di;
            if (_M_end_of_file(_is)) { break; }
            _i = _M_get_positive_offset(_di, true);
            const_iterator _p = this->_container.cbegin() + _i;
            this->_M_reg_erase(_p, _log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__CLEAR__: {
            this->_M_reg_clear(_log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__SIZE__: {
            this->_M_reg_size(_log);
        }; break;
        case base::__PRINT__: {
            this->_M_print_container();
        }; break;
        case base::__QUIT__: {
            _M_reset_cin(_is);
            return;
        }; break;
        case base::__PAUSE__: {
            _S_pause();
        }; break;
        default: break;
        }
        _op.clear();
        _M_reset_cin(_is);
        if (auto _ret = this->_M_reg_check()) {
            ASP_ERR("error(%d) in container.\n", _ret);
            break;
        }
        std::cout << std::flush;
    }
    _M_reset_cin(_is);
};
template <typename _SC> void debug_seq_container<_SC>::init_stream(std::stringstream& _is, size_type _n) {
    const int _max_key_value = 31;
    const int _modify_func = (_push_front != nullptr && _pop_front != nullptr) ? 3 : 2;
    const size_type _n_bak = _n;
    std::array<double, 3> _add_array {0.6, 0.3, 0.1};
    std::array<double, 3> _del_array {0.3, 0.6, 0.1};
    std::array<double, 3> _normal_array {0.45, 0.45, 0.1};
    while (_n--) {
        size_type _oper = 0;
        if (_n > 2 * _n_bak / 3) {
            _oper = asp::_S_random_unsigned<3>(_add_array);
        }
        else if (_n > _n_bak / 3) {
            _oper = asp::_S_random_unsigned<3>(_normal_array);
        }
        else {
            _oper = asp::_S_random_unsigned<3>(_del_array);
        }
        if (_oper == 0) { // add
            _oper = rand() % _modify_func;
            _is << (_oper == 0 ? "i" : (_oper == 1 ? "push" : "push_front")) << ' ';
            if (_oper == 0) {
                size_type _i;
                _i = rand() % (this->_container.size() + 1);
                _is << _i << ' ';
            }
            value_type _v = rand() % _max_key_value;
            _is << _v << ' ';
        }
        else if (_oper == 1) { // del
            _oper = rand() % _modify_func;
            _is << (_oper == 0 ? "e" : (_oper == 1 ? "pop" : "pop_front")) << ' ';
            if (_oper == 0) {
                size_type _i;
                _i = rand() % (this->_container.size() + 1);
                _is << _i << ' ';
            }
        }
        else { // query
            int op = rand() % 2;
            _is << (op == 0 ? "clear" : "size") << ' ';
        }
    }
    _is << 'p' << ' ';
};

template <typename _AC> void debug_asso_container<_AC>::demo_from_istream(std::istream& _is, bool _log, bool _print_container) {
    key_type _k;
    // value_type _v;
    mapped_type _m;
    std::string _op;
    while (!_is.eof()) {
        _is >> _op;
        if (_M_end_of_file(_is)) { break; }
        operator_id _id = this->_M_get_operator_id(_op);
        switch (_id) {
        case base::__ADD__: {
            value_type _v = this->_M_get_value_from(_is);
            if (_M_end_of_file(_is)) { break; }
            this->_M_reg_insert(_v, _log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        // case base::__SET__: {
        //     _is >> _k;
        //     if (_M_end_of_file(_is)) { break; }
        //     _is >> _m;
        //     if (_M_end_of_file(_is)) { break; }
        //     this->_M_reg_set(_k, _m, true);
        //     this->_M_print_container();
        // }; break;
        case base::__DELETE__: {
            _is >> _k;
            if (_M_end_of_file(_is)) { break; }
            this->_M_reg_erase(_k, _log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__CLEAR__: {
            this->_M_reg_clear(_log);
            if (_log && _print_container) this->_M_print_container();
        }; break;
        case base::__COUNT__: {
            _is >> _k;
            if (_M_end_of_file(_is)) { break; }
            this->_M_reg_count(_k, _log);
        }; break;
        case base::__FIND__: {
            _is >> _k;
            if (_M_end_of_file(_is)) { break; }
            this->_M_reg_find(_k, _log);
        }; break;
        case base::__SIZE__: {
            this->_M_reg_size(_log);
        }; break;
        case base::__PRINT__: {
            this->_M_print_container();
        }; break;
        case base::__QUIT__: {
            _M_reset_cin(_is);
            return;
        }; break;
        case base::__PAUSE__: {
            _S_pause();
        }; break;
        default: break;
        }
        _op.clear();
        _M_reset_cin(_is);
        if (auto _ret = this->_M_reg_check()) {
            ASP_ERR("error(%d) in container.\n", _ret);
            break;
        }
        std::cout << std::flush;
    }
    _M_reset_cin(_is);
};
template <typename _AC> void debug_asso_container<_AC>::init_stream(std::stringstream& _is, size_type _n) {
    /**
     * 
    */
    const int _max_key_value = 31;
    const int _max_value = 255;
    srand((int)time(nullptr));
    while (_n--) {    
        int _oper = rand() % 7;
        _oper /= 3;
        if (_oper == 0) { // add
            _is << 'a';
            key_type _k;
            _k = rand() % _max_key_value;
            _is << ' ' << _k << ' ';
            if (!_b_kv_self) {
                mapped_type _m;
                _m = rand() % _max_value;
                _is << _m << ' ';
            }
        }
        else if (_oper == 1) { // del
            _is << 'd';
            key_type _k;
            _k = rand() % _max_key_value;
            _is << ' ' << _k << ' ';
        }
        else { // query
            int op = rand() % 4;
            _is << (op == 0 ? "c" : (op == 1 ? "f" : (op == 2 ? "clear" : "size")));
            key_type _k;
            _k = rand() % _max_key_value;
            _is << ' ' << _k << ' ';
        }
    }
    _is << 'p' << ' ';
};


/// _M_reg_function
template <typename _C> int debug_base<_C>::_M_reg_check() const {
    int _out_check_ret = 0;
    if (this->_out_check != nullptr) {
        _out_check_ret = _out_check(this->_container);
    }
    int _inner_check_ret = 0;
    if (this->_check != nullptr) {
        _inner_check_ret = (this->_container.*this->_check)();
    }
    return _out_check_ret + _inner_check_ret;
};
template <typename _C> void debug_base<_C>::_M_reg_clear(bool _log) {
    if (this->_clear != nullptr) {
        (this->_container.*this->_clear)();
        if (_log) {
            std::cout << "clear" << std::endl;;
        }
    }
};
template <typename _C> void debug_base<_C>::_M_reg_size(bool _log) const {
    if (this->_size != nullptr) {
        if (_log) {
            std::cout << "size() = " << (this->_container.*this->_size)() << std::endl;;
        }
        else {
            (this->_container.*this->_size)();
        }
    }
};
template <typename _C> void debug_seq_container<_C>::_M_reg_push_back(const value_type& _v, bool _log) {
    if (this->_push_back != nullptr) {
        (this->_container.*_push_back)(_v);
        if (_log) {
            std::cout << "push_back(" << _v << ")" << std::endl;
        }
    }
};
template <typename _C> void debug_seq_container<_C>::_M_reg_pop_back(bool _log) {
    if (this->_push_back != nullptr) {
        (this->_container.*_pop_back)();
        if (_log) {
            std::cout << "pop_back" << std::endl;
        }
    }
};
template <typename _C> void debug_seq_container<_C>::_M_reg_push_front(const value_type& _v, bool _log) {
    if (this->_push_front != nullptr) {
        (this->_container.*_push_front)(_v);
        if (_log) {
            std::cout << "push_front(" << _v << ")" << std::endl;
        }
    }
};
template <typename _C> void debug_seq_container<_C>::_M_reg_pop_front(bool _log) {
    if (this->_pop_front != nullptr) {
        (this->_container.*_pop_front)();
        if (_log) {
            std::cout << "pop_front" << std::endl;
        }
    }
};
template <typename _C> void debug_seq_container<_C>::_M_reg_insert(const_iterator _i, const value_type& _v, bool _log) {
    if (this->_insert != nullptr) {
        if (_log) {
            std::cout << "*insert(&"<< this->_M_string_from_iterator(_i) << ", " << _v << ""  << ")";
        }
        auto _p = (this->_container.*_insert)(_i, _v);
        if (_log) {
            std::cout << " = " << this->_M_string_from_iterator(_p) << ")" << std::endl;
        }
    }
};
template <typename _C> void debug_seq_container<_C>::_M_reg_erase(const_iterator _i, bool _log) {
    if (this->_erase != nullptr) {
        if (_log) {
            std::cout << "*erase(" << this->_M_string_from_iterator(_i) << ")";
        }
        auto _p = (this->_container.*_erase)(_i);
        if (_log) {
            std::cout << " = (" << this->_M_string_from_iterator(_p) << ")" << std::endl;
        }
    }
};
template <typename _C> void debug_asso_container<_C>::_M_reg_insert(const value_type& _v, bool _log) {
    if (this->_insert != nullptr) {
        auto _r = (this->_container.*_insert)(_v);
        bool _i_status = typename container_type::insert_status()(_r);
        auto _p = typename container_type::ext_iterator()(_r);
        if (_log) {
            const auto _str = this->_M_string_from_iterator(_p);
            if (!_i_status) {
                std::cout << "fail to add, (" << _str << ") existed" << std::endl;
            }
            else {
                // const key_type _k = typename container_type::_ExtractKey()(_v);
                // const mapped_type _m = typename container_type::_ExtractValue()(_v);
                std::cout << "*add(...) = " << _str << std::endl;
            }
        }
    }
};
// template <typename _C> void debug_asso_container<_C>::_M_reg_set(const key_type& _k, const mapped_type& _v, bool _log) {
//     if (this->_set != nullptr) {
//         auto _r = (this->_container.*_set)(_k, _v);
//         auto _p = typename container_type::_ExtractIterator()(_r);
//         if (_log) {
//             std::cout << "*set(@" << _k << ", " << _v << ") = " << this->_M_string_from_iterator(_p) << std::endl;
//         }
//     }
// };
template <typename _C> void debug_asso_container<_C>::_M_reg_erase(const key_type& _k, bool _log) {
    if (this->_erase != nullptr) {
        auto _cnt = (this->_container.*_erase)(_k);
        if (_log) {
            std::cout << "erase(" << _k << ") = " << _cnt << std::endl;
        }
    }
};
template <typename _C> void debug_asso_container<_C>::_M_reg_count(const key_type& _k, bool _log) const {
    if (this->_count != nullptr) {
        auto _s = (this->_container.*this->_count)(_k);
        if (_log) {
            std::cout << "count(" << _k << ") = " << _s << std::endl;
        }
    }
};
template <typename _C> void debug_asso_container<_C>::_M_reg_find(const key_type& _k, bool _log) const {
    if (this->_count != nullptr) {
        auto _p = (this->_container.*this->_find)(_k);
        if (_log) {
            std::cout << "*find(" << _k << ") = " << _p << std::endl;
        }
    }
};

/// conversion between string & iterator
template <typename _C> auto debug_base<_C>::_M_string_from_iterator(const_iterator _i) const
-> std::string {
    std::stringstream _ss;
    if (_i != this->_container.cend()) {
        // if (has_operator_out<std::ostream&, const const_iterator&>::_value) {
            _ss << _i;
        // }
        // else {
        //     _ss << *_i;
        // }
    }
    else {
        _ss << "null";
    }
    return _ss.str();
};

template <typename _C> auto debug_seq_container<_C>::_M_get_positive_offset(difference_type _i, bool _del) const
-> size_type {
    if (_i >= 0) {
        return std::min(this->_container.size(), size_type(_i));
    }
    else {
        return std::max(difference_type(0), difference_type(this->_container.size() + _i) + (_del ? 0 : 1));
    }
};

/// input
template <typename _T1, typename _T2> std::istream& operator>>(std::istream& is, std::pair<_T1, _T2>& _p) {
    is >> _p.first >> _p.second;
    return is;
};
template <typename _C> auto debug_asso_container<_C>::_M_get_value_from(std::istream& is) const
-> value_type {
    asp::conditional_t<asp::is_same<key_type, value_type>::value, value_type, std::pair<key_type, mapped_type>> __v;
    is >> __v;
    return value_type(__v);
};

};

#endif