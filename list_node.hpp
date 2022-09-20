#ifndef _ASP_LIST_NODE_HPP_
#define _ASP_LIST_NODE_HPP_

#include <utility>

#include "node.hpp"

namespace asp {
template <typename _Tp> struct list_node;

template <typename _Tp> struct list_node : public node<_Tp> {
    // typedef typename node<_Tp>::value_type value_type;
    using value_type = typename node<_Tp>::value_type;
    using pointer = typename node<_Tp>::pointer;
    using reference = typename node<_Tp>::reference;
    typedef list_node<value_type> lnode;
    list_node(): node<value_type>() {}
    list_node(const value_type& n): node<value_type>(n) {}
    list_node(const lnode& rhs): node<value_type>(rhs), prev(rhs.prev), next(rhs.next) {}
    list_node(lnode&& rhs): node<value_type>(std::move(rhs)), prev(rhs.prev), next(rhs.next) {}
    virtual lnode& operator=(const lnode& rhs) {
        node<value_type>::operator=(rhs);
        prev = rhs.prev;
        next = rhs.next;
        return *this;
    }
    virtual lnode& operator=(lnode&& rhs) {
        node<value_type>::operator=(std::move(rhs));
        prev = rhs.prev;
        next = rhs.next;
        return *this;
    }
    virtual ~list_node() {
        reset();
    }

    void reset() { prev = nullptr; next = nullptr; }

    // hook @this before @p
    void hook(lnode* const p) {
        this->prev = p->prev;
        this->next = p;
        p->prev->next = this;
        p->prev = this;
    }
    void unhook() {
        this->prev->next = this->next;
        this->next->prev = this->prev;
        this->reset();
    }

    // append @p after @this
    lnode* append(lnode* const p) {
        p->next = this->next;
        p->prev = this;
        this->next->prev = p;
        this->next = p;
        return this;
    }

    lnode* prev = nullptr;
    lnode* next = nullptr;
};

template <typename _Tp> struct list_iterator;

template <typename _Tp> struct list_iterator {
    typedef asp::bidirectional_iterator_tag iterator_category;
    typedef list_iterator<_Tp> self;
    typedef list_node<_Tp> lnode;
    typedef _Tp value_type;
    typedef asp::difference_type difference_type;

    list_iterator() {}
    list_iterator(lnode* n): _ptr(n) {}
    list_iterator(const self& rhs): _ptr(rhs._ptr) {}
    self& operator=(const self& rhs) {
        _ptr = rhs._ptr;
        return *this;
    }

    self _const_cast() const {
        return *this;
    }

    value_type& operator*() { return _ptr->val(); }
    // const value_type& operator*() const { return _ptr->val(); }
    value_type* operator->() { return _ptr->valptr(); }
    // const value_type* operator->() const { return _ptr->valptr(); }
    self* base() { return this; }

    self& operator++() {
        _ptr = _ptr->next;
        return *this;
    }
    self operator++(int) {
        self _tmp = *this;
        _ptr = _ptr->next;
        return _tmp;
    }
    self& operator--() {
        _ptr = _ptr->prev;
        return *this;
    }
    self operator--(int) {
        self _tmp = *this;
        _ptr = _ptr->prev;
        return _tmp;
    }
    self operator+(int n) {
        self _tmp = *this;
        while (n--) {
            _tmp._ptr = _tmp._ptr->next;
        }
        return _tmp;
    }
    self operator-(int n) {
        self _tmp = *this;
        while (n--) {
            _tmp._ptr = _tmp._ptr->prev;
        }
        return _tmp;
    }
    friend bool operator==(const self& x, const self& y) {
        return x._ptr == y._ptr;
    }
    friend bool operator!=(const self& x, const self& y) {
        return x._ptr != y._ptr;
    }

// private:
    lnode* _ptr = nullptr;
};

template <typename _Tp> struct list_const_iterator;

template <typename _Tp> struct list_const_iterator {
    typedef list_const_iterator<_Tp> self;
    typedef list_iterator<_Tp> iterator;
    typedef list_node<_Tp> lnode;
    typedef _Tp value_type;

    list_const_iterator() {}
    list_const_iterator(const lnode* n): _ptr(n) {}
    list_const_iterator(const self& rhs): _ptr(rhs._ptr) {}
    list_const_iterator(const iterator& rhs): _ptr(rhs._ptr) {}
    self& operator=(const self& rhs) {
        _ptr = rhs._ptr;
    }

    iterator _const_cast() const {
        return iterator(const_cast<lnode*>(_ptr));
    }

    const value_type& operator*() const { return _ptr->val(); }
    const value_type* operator->() const { return _ptr->valptr(); }
    const self* base() const { return this; }
    const self& operator++() {
        _ptr = _ptr->next;
        return *this;
    }
    self operator++(int) {
        self _tmp = *this;
        _ptr = _ptr->next;
        return _tmp;
    }
    const self& operator--() {
        _ptr = _ptr->prev;
        return *this;
    }
    self operator--(int) {
        self _tmp = *this;
        _ptr = _ptr->prev;
        return _tmp;
    }
    self operator+(int n) {
        self _tmp = *this;
        while (n--) {
            _tmp._ptr = _tmp._ptr->next;
        }
        return _tmp;
    }
    self operator-(int n) {
        self _tmp = *this;
        while (n--) {
            _tmp._ptr = _tmp._ptr->prev;
        }
        return _tmp;
    }
    friend bool operator==(const self& x, const self& y) {
        return x._ptr == y._ptr;
    }
    friend bool operator!=(const self& x, const self& y) {
        return x._ptr != y._ptr;
    }

// private:
    const lnode* _ptr = nullptr;
};

};


#endif // !#ifndef _ASP_LIST_NODE_HPP_
