#ifndef _ASP_SKIP_LIST_NODE_HPP_
#define _ASP_SKIP_LIST_NODE_HPP_

#include "basic_param.hpp"
#include "node.hpp"

namespace asp {

template <typename _Value> struct skip_list_node;
template <typename _Tp> struct skip_list_iterator;
template <typename _Tp> struct skip_list_const_iterator;

template <typename _Value> struct skip_list_node : public node<_Value> {
    typedef node<_Value> base;
    typedef skip_list_node<_Value> self;
    typedef typename base::value_type value_type;
    typedef typename base::pointer pointer;
    typedef typename base::reference reference;

    skip_list_node() : base() {}
    skip_list_node(const self& _n) : base(_n) {}
    skip_list_node(self&& _n) : base(std::move(_n)) {}
    skip_list_node(const value_type& _v) : base(_v) {}
    template <typename... _Args> skip_list_node(_Args&&... _args) : base(std::forward<_Args>(_args)...) {}
    virtual ~skip_list_node() {}

    /**
     * @brief next pointers.
     *   _next[0] is the real next pointer in list, the others are index pointer.
    */
    self* _next[1] = { nullptr };
    self* _prev = nullptr;
    size_type _height = 1;

    void reset() {
        _prev = nullptr;
        _next[0] = nullptr;
    }
};

template <typename _Tp> struct skip_list_iterator {
    typedef asp::bidirectional_iterator_tag iterator_category;
    typedef skip_list_iterator<_Tp> self;
    typedef skip_list_node<_Tp> node_type;
    typedef typename node_type::value_type value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef asp::difference_type difference_type;

    node_type* _ptr = nullptr;

    skip_list_iterator() {}
    skip_list_iterator(node_type* _n) : _ptr(_n) {}
    value_type& operator*() const { return _ptr->val(); }
    value_type* operator->() const { return _ptr->valptr(); }
    self& operator++() { _ptr = _ptr->_next[0]; return *this; }
    self operator++(int) { self _ret = *this; _ptr = _ptr->_next[0]; return _ret; }
    self& operator--() { _ptr = _ptr->_prev;  return *this; }
    self operator--(int) { self _ret = *this; _ptr = _ptr->_prev; return _ret; }
    operator bool() const { return _ptr != nullptr; }
    friend bool operator==(const self& _x, const self& _y) {return _x._ptr == _y._ptr; }
    friend bool operator!=(const self& _x, const self& _y) {return _x._ptr != _y._ptr; }
    template <typename _T> friend std::ostream& operator<<(std::ostream& os, const skip_list_iterator<_T>& _sli);
};
template <typename _Tp> struct skip_list_const_iterator {
    typedef asp::bidirectional_iterator_tag iterator_category;
    typedef skip_list_const_iterator<_Tp> self;
    typedef skip_list_iterator<_Tp> iterator;
    typedef skip_list_node<_Tp> node_type;
    typedef typename node_type::value_type value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef asp::difference_type difference_type;

    const node_type* _ptr = nullptr;

    skip_list_const_iterator() {}
    skip_list_const_iterator(const node_type* _n) : _ptr(_n) {}
    skip_list_const_iterator(const iterator& _i) : _ptr(_i._ptr) {}
    const value_type& operator*() const { return _ptr->val(); }
    const value_type* operator->() const { return _ptr->valptr(); }
    self& operator++() { _ptr = _ptr->_next[0]; return *this; }
    self operator++(int) { self _ret = *this; _ptr = _ptr->_next[0]; return _ret; }
    self& operator--() { _ptr = _ptr->_prev;  return *this; }
    self operator--(int) { self _ret = *this; _ptr = _ptr->_prev; return _ret; }
    operator bool() const { return _ptr != nullptr; }
    friend bool operator==(const self& _x, const self& _y) {return _x._ptr == _y._ptr; }
    friend bool operator!=(const self& _x, const self& _y) {return _x._ptr != _y._ptr; }
    template <typename _T> friend std::ostream& operator<<(std::ostream& os, const skip_list_const_iterator<_T>& _sli);
};

};

#endif  // _ASP_SKIP_LIST_NODE_HPP_