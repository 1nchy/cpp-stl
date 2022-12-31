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

    check_fptr _check;
    clear_fptr _clear;
    size_fptr _size;

    container_type _container;

    debug_base() = default;
    debug_base(check_fptr _f) : _check(_f) {}
    virtual ~debug_base() = default;

    virtual void demo() = 0;

    // std::string _M_string_from_iterator(iterator _i) const;
    std::string _M_string_from_iterator(const_iterator _i) const;
};

template <typename _SeqContainer> struct debug_seq_container : public debug_base<_SeqContainer> {
    typedef debug_base<_SeqContainer> base;
    typedef typename base::container_type container_type;
    typedef typename base::check_fptr check_fptr;
    typedef typename base::value_type value_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
/// container function typedef
    typedef void (container_type::*void_modify_fptr)(const value_type&);
    typedef iterator (container_type::*insert_fptr)(const_iterator, const value_type&);
    typedef iterator (container_type::*earse_fptr)(const_iterator);
    typedef typename base::size_fptr size_fptr;
/// member
    std::unordered_map<int, void_modify_fptr> _void_modify_map;
    insert_fptr _insert_map;
    earse_fptr _erase_map;

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
/// container function typedef
    typedef iterator (container_type::*insert_fptr)(const value_type&);
    typedef iterator (container_type::*earse_fptr)(const key_type&);
    typedef iterator (container_type::*find_fptr)(const key_type&);
    typedef const_iterator (container_type::*cfind_fptr)(const key_type&) const;
    typedef size_type (container_type::*count_fptr)(const key_type&) const;
    typedef typename base::size_fptr size_fptr;
/// member
    insert_fptr _insert;
    earse_fptr _erase;
    find_fptr _find;
    cfind_fptr _cfind;
    count_fptr _count;

/// constructor
    debug_asso_container() = default;
    virtual ~debug_asso_container() = default;

/// demo function
    void demo() override;
};


template <typename _SC> void debug_seq_container<_SC>::demo() {

};
template <typename _AC> void debug_asso_container<_AC>::demo() {
    key_type _k;
    value_type _v;
    char _op;
    std::cout << '[' << typeid(asp::decay_t<_AC>).name() << "]:" << std::endl;
    while (!std::cin.eof()) {
        std::cin >> _op;
        switch (_op) {
        case 'i': {
            std::cin>> _v;
            if (_insert != nullptr) {
                auto _p = (this->_container.*_insert)(_v);
                std::cout << "insert(" << _v << ") = " << this->_M_string_from_iterator(_p) << std::endl;
            }
            std::cout << this->_container << std::endl;
        }; break;
        case 'e': {
            std::cin >> _k;
            if (_erase != nullptr) {
                auto _p = (this->_container.*_erase)(_k);
                std::cout << "erase(" << _k << ") = " << this->_M_string_from_iterator(_p) << std::endl;
            }
            std::cout << this->_container << std::endl;
        }; break;
        case 'c': {
            if (this->_clear != nullptr) {
                (this->_container.*this->_clear)();
            }
            std::cout << this->_container << std::endl;
        }; break;
        
        default:

            break;
        }

    }
};


/// output
// template <typename _C> auto debug_base<_C>::_M_string_from_iterator(iterator _i) const
// -> std::string {
//     std::stringstream _ss;
//     if (_i != this->_container.end()) {
//         _ss << *_i;
//     }
//     else {
//         _ss << "null";
//     }
//     return _ss.str();
// };
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