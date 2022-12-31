#ifndef _ASP_DEBUG_HPP_
#define _ASP_DEBUG_HPP_

#include <ostream>
#include <iostream>
#include <sstream>
#include <functional>
#include <map>
#include <unordered_map>

#include "basic_param.hpp"
#include "log_utils.hpp"
#include "type_traits.hpp"

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

template <typename _Container> struct debug_base {
    typedef _Container container_type;
    typedef typename _Container::value_type value_type;
    typedef typename _Container::iterator iterator;
    typedef typename _Container::const_iterator const_iterator;

    typedef bool (*check_fptr)(const _Container&);
    typedef void (_Container::*clear_fptr)();

    typedef size_type (_Container::*size_fptr)() const;

    check_fptr _check = nullptr;
    clear_fptr _clear = nullptr;
    size_fptr _size = nullptr;

    container_type _container;

    debug_base() = default;
    debug_base(check_fptr _f) : _check(_f) {}
    virtual ~debug_base() = default;

    virtual void demo() = 0;
    void _M_print_container() const { std::cout << _container << std::endl;}

    bool _M_reg_check() const;
    void _M_reg_clear();
    void _M_reg_size(bool _log = false) const;

    // std::string _M_string_from_iterator(iterator _i) const;
    std::string _M_string_from_iterator(const_iterator _i) const;

    enum operator_id {
        __PUSH_BACK__, __POP_BACK__,
        __PUSH_FRONT__, __POP_FRONT__,
        __INSERT__, __ERASE__,
        __CLEAR__, __COUNT__,
        __SHOW__,
        __NONE__,
    };

    const std::unordered_map<std::string, operator_id> _operator_map = {
        {"push_back", __PUSH_BACK__}, {"pop_back", __POP_BACK__},
        {"push", __PUSH_BACK__}, {"pop", __POP_BACK__},
        {"pushb", __PUSH_BACK__}, {"popb", __POP_BACK__},
        {"push_front", __PUSH_FRONT__}, {"pop_front", __POP_FRONT__},
        {"pushf", __PUSH_FRONT__}, {"popf", __POP_FRONT__},
        {"insert", __INSERT__}, {"erase", __ERASE__},
        {"clear", __CLEAR__}, {"count", __COUNT__},
        {"i", __INSERT__}, {"e", __ERASE__},
        {"l", __CLEAR__}, {"c", __COUNT__},
        {"print", __SHOW__}, {"s", __SHOW__}, {"show", __SHOW__}
    };
    
    operator_id _M_get_operator_id(const std::string& _op) {
        auto _it = _operator_map.find(_op);
        return _it != _operator_map.cend() ? _it->second : __NONE__;
    }
};

template <typename _SeqContainer> struct debug_seq_container : public debug_base<_SeqContainer> {
    typedef debug_base<_SeqContainer> base;
    typedef typename base::container_type container_type;
    typedef typename base::check_fptr check_fptr;
    typedef typename base::value_type value_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef typename base::operator_id operator_id;
/// container function typedef
    typedef void (container_type::*void_modify_fptr)(const value_type&);
    typedef iterator (container_type::*insert_fptr)(const_iterator, const value_type&);
    typedef iterator (container_type::*earse_fptr)(const_iterator);
    typedef typename base::size_fptr size_fptr;
/// member
    void_modify_fptr _push_back = nullptr;
    void_modify_fptr _pop_back = nullptr;
    void_modify_fptr _push_front = nullptr;
    void_modify_fptr _pop_front = nullptr;
    insert_fptr _insert_map = nullptr;
    earse_fptr _erase_map = nullptr;

/// demo function
    void demo() override;
};

template <typename _AssoContainer> struct debug_asso_container : public debug_base<_AssoContainer> {
    typedef debug_base<_AssoContainer> base;
    typedef typename base::container_type container_type;
    typedef typename base::check_fptr check_fptr;
    typedef typename base::value_type value_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef typename container_type::key_type key_type;
    typedef typename base::operator_id operator_id;
/// container function typedef
    typedef iterator (container_type::*insert_fptr)(const value_type&);
    typedef iterator (container_type::*earse_fptr)(const key_type&);
    typedef iterator (container_type::*find_fptr)(const key_type&);
    typedef const_iterator (container_type::*cfind_fptr)(const key_type&) const;
    typedef size_type (container_type::*count_fptr)(const key_type&) const;
    typedef typename base::size_fptr size_fptr;
/// member
    insert_fptr _insert = nullptr;
    earse_fptr _erase = nullptr;
    find_fptr _find = nullptr;
    cfind_fptr _cfind = nullptr;
    count_fptr _count = nullptr;

/// constructor
    debug_asso_container() = default;
    virtual ~debug_asso_container() = default;

/// register function
    void _M_reg_insert(const value_type& _v, bool _log = false);
    void _M_reg_erase(const key_type& _k, bool _log = false);
    void _M_reg_count(const key_type& _k, bool _log = false) const;

/// demo function
    void demo() override;
};

static bool _M_end_of_file() {
    return std::cin.eof() || std::cin.fail();
};
static void _M_reset_cin() {
    std::cin.clear(); // std::cin.sync();
}

template <typename _SC> void debug_seq_container<_SC>::demo() {
    size_type _i; // iterator_index
    value_type _v;
    std::string _op;
    std::cout << '[' << typeid(asp::decay_t<_SC>).name() << "]:" << std::endl;
    while (!std::cin.eof()) {
        std::cin >> _op;
        if (_M_end_of_file()) { break; }
        switch (_op) {
        case "push":
        case "push_back": {
            std::cin >> _v;
            if (_M_end_of_file()) { break; }
            // if (_)
        }; break;
        }
    }
};
template <typename _AC> void debug_asso_container<_AC>::demo() {
    key_type _k;
    value_type _v;
    std::string _op;
    std::cout << '[' << typeid(asp::decay_t<_AC>).name() << "]:" << std::endl;
    while (!std::cin.eof()) {
        std::cin >> _op;
        operator_id _id = this->_M_get_operator_id(_op);
        if (_M_end_of_file()) { break; }
        switch (_id) {
        case base::__INSERT__: {
            std::cin>> _v;
            if (_M_end_of_file()) { break; }
            this->_M_reg_insert(_v, true);
            this->_M_print_container();
        }; break;
        case base::__ERASE__: {
            std::cin >> _k;
            if (_M_end_of_file()) { break; }
            this->_M_reg_erase(_k, true);
            this->_M_print_container();
        }; break;
        case base::__CLEAR__: {
            this->_M_reg_clear();
            this->_M_print_container();
        }; break;
        case base::__COUNT__: {
            std::cin >> _k;
            if (_M_end_of_file()) { break; }
            this->_M_reg_count(_k, true);
        }; break;
        case base::__SHOW__: {
            this->_M_print_container();
        }; break;
        default:
            break;
        }
        _op.clear();
        _M_reset_cin();
    }
};


/// _M_reg_function
template <typename _C> bool debug_base<_C>::_M_reg_check() const {
    if (this->_check != nullptr) {
        return _check(this->_container);
    }
    return true;
};
template <typename _C> void debug_base<_C>::_M_reg_clear() {
    if (this->_clear != nullptr) {
        (this->_container.*this->_clear)();
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
template <typename _C> void debug_asso_container<_C>::_M_reg_insert(const value_type& _v, bool _log) {
    if (this->_insert != nullptr) {
        if (_log) {
            std::cout << "insert(" << _v << ") = "
             << this->_M_string_from_iterator((this->_container.*_insert)(_v)) << std::endl;
        }
        else {
            (this->_container.*_insert)(_v);
        }
    }
};
template <typename _C> void debug_asso_container<_C>::_M_reg_erase(const key_type& _k, bool _log) {
    if (this->_erase != nullptr) {
        if (_log) {
            std::cout << "erase(" << _k << ") = "
             << this->_M_string_from_iterator((this->_container.*_erase)(_k)) << std::endl;
        }
        else {
            (this->_container.*_erase)(_k);
        }
    }
};
template <typename _C> void debug_asso_container<_C>::_M_reg_count(const key_type& _k, bool _log) const {
    if (this->_count != nullptr) {
        if (_log) {
            std::cout << "count(" << _k << ") = " << (this->_container.*this->_count)(_k) << std::endl;
        }
        else {
            (this->_container.*this->_count)(_k);
        }
    }
};

/// conversion between string & iterator
template <typename _C> auto debug_base<_C>::_M_string_from_iterator(const_iterator _i) const
-> std::string {
    std::stringstream _ss;
    if (_i != this->_container.cend()) {
        _ss << *_i;
    }
    else {
        _ss << "null";
    }
    return _ss.str();
};
};

#endif