#ifndef _ASP_DEBUG_HPP_
#define _ASP_DEBUG_HPP_

#include <ostream>
#include <functional>
#include <map>
#include <unordered_map>

#include "basic_param.hpp"

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

    typedef decltype(std::function<bool(const _Constainer& _l) const>) check_fptr;
    typedef void (_Container::*clear_fptr)();

    typedef size_type (_Container::*size_fptr)() const;

    check_fptr _check;
    clear_fptr _clear;
    size_fptr _size;

    debug(check_fptr _f) : _check(_f) {}
    virtual ~debug() = default;
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
    std::unordered_map<int, insert_fptr> _insert_map;
    std::unordered_map<int, earse_fptr> _erase_map;
};

template <typename _AssoContainer> struct debug_asso_container : public debug_base<_AssoContainer> {
    typedef debug_base<_AssoContainer> base;
    typedef typename base::container_type container_type;
    typedef typename base::check_fptr check_fptr;
    typedef typename base::value_type value_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef typename container_type::key_value key_value;
/// container function typedef
    typedef iterator (container_type::*insert_fptr)(const key_value&, const value_type&);
    typedef iterator (container_type::*earse_fptr)(const key_value&);
    typedef iterator (container_type::*find_fptr)(const key_value&);
    typedef const_iterator (container_type::*cfind_fptr)(const key_value&) const;
    typedef size_type (container_type::*count_fptr)(const key_value&) const;
    typedef typename base::size_fptr size_fptr;
/// member
    std::unordered_map<int, insert_fptr> _insert_map;
    std::unordered_map<int, earse_fptr> _erase_map;
    find_fptr _find;
    cfind_fptr _cfind;
    count_fptr _count;
};

};

#endif