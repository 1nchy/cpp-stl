#ifndef _ASP_LIST_HPP_
#define _ASP_LIST_HPP_

#include <ostream>

#include "list_node.hpp"

namespace asp {
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> class list;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> struct list_alloc;

template <typename _Tp, typename _Alloc> struct list_alloc : public _Alloc {
    typedef list_node<_Tp> node_type;
    typedef _Alloc elt_allocator_type;
    typedef std::allocator_traits<elt_allocator_type> elt_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<node_type> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_alloc_traits;

    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type*>(this); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<const elt_allocator_type*>(this); }
    node_allocator_type _M_get_node_allocator() const { return node_allocator_type(_M_get_elt_allocator()); }

    node_type* _M_allocate_node(const node_type& _x) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, _x.val());
        return _p;
    }
    template <typename... _Args> node_type* _M_allocate_node(_Args&&... _args) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        auto _ptr = node_alloc_traits::allocate(_node_alloc, 1);
        node_type* _p = std::addressof(*_ptr);
        node_alloc_traits::construct(_node_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_node(node_type* _p) {
        node_allocator_type _node_alloc = _M_get_node_allocator();
        node_alloc_traits::destroy(_node_alloc, _p);
        node_alloc_traits::deallocate(_node_alloc, _p, 1);
    }
};

template <typename _Tp, typename _Alloc> class list : public list_alloc<_Tp, _Alloc> {
    typedef list<_Tp, _Alloc> self;
    typedef list_alloc<_Tp, _Alloc> base;
public:
    typedef typename base::node_type node_type;
    typedef typename node_type::value_type value_type;
    typedef typename node_type::pointer pointer;
    typedef typename node_type::reference reference;
    typedef list_iterator<value_type> iterator;
    typedef list_const_iterator<value_type> const_iterator;
    list() {
        _M_init_mark();
    }
    list(const self& _l) : m_element_count(_l.m_element_count) {
        _M_init_mark();
        this->_M_assign(_l);
    }
    self& operator=(const self& _l) {
        if (&_l == this) return *this;
        clear();
        _M_init_mark();
        this->_M_assign(_l);
        return *this;
    }
    virtual ~list() {
        node_type* p = mark.next;
        while (p != &mark) {
            auto pnext = p->next;
            // delete p;
            this->_M_deallocate_node(p);
            p = pnext;
        }
    };

    const value_type& front() const { return mark.next->val(); }
    value_type& front() { return mark.next->val(); }
    const value_type& back() const { return mark.prev->val(); }
    value_type& back() { return mark.prev->val(); }
    size_type size() const { return m_element_count; }
    bool empty() const { return size() == 0; }

    /// iterator
    iterator begin() { return iterator(mark.next); }
    const_iterator cbegin() const { return const_iterator(mark.next); }
    iterator end() { return iterator(&mark); }
    const_iterator cend() const { return const_iterator(&mark); }

    void push_front(const value_type& e) { // to head
        node_type* p = this->_M_allocate_node(e);
        if (p == nullptr) {
            return;
        }
        p->hook(mark.next);
        ++m_element_count;
    }
    void pop_front() {
        if (empty()) {
            return;
        }
        node_type* p = mark.next;
        p->unhook();
        this->_M_deallocate_node(p);
        --m_element_count;
    }
    void push_back(const value_type& e) { // to tail
        node_type* p = this->_M_allocate_node(e);
        if (p == nullptr) {
            return;
        }
        p->hook(&mark);
        ++m_element_count;
    }
    void pop_back() {
        if (empty()) {
            return;
        }
        node_type* p = mark.prev;
        p->unhook();
        this->_M_deallocate_node(p);
        --m_element_count;
    }
    iterator insert(const_iterator pos, const value_type& e) {
        node_type* p = this->_M_allocate_node(e);
        p->hook(pos._const_cast()._ptr);
        ++m_element_count;
        return iterator(p);
    }
    iterator erase(const_iterator pos) {
        iterator _ret = iterator(pos._const_cast()._ptr->next);
        node_type* p =pos._const_cast()._ptr;
        if (p == &mark) {
            return end();
        }
        p->unhook();
        --m_element_count;
        this->_M_deallocate_node(p);
        return _ret;
    }
    iterator erase(const_iterator first, const_iterator last) {
        iterator _ret = iterator(last._const_cast()._ptr);
        node_type* p = first._const_cast()._ptr;
        while (p != last._ptr) {
            auto _tmp = p->next;
            p->unhook();
            this->_M_deallocate_node(p);
            --m_element_count;
            p = _tmp;
        }
        return _ret;
    }
    void clear() {
        if (empty()) return;
        node_type* prev = nullptr;
        node_type* p = mark.next;
        for (; p != nullptr && p != &mark;) {
            prev = p;
            p = p->next;
            this->_M_deallocate_node(prev);
        }
        _M_init_mark();
        m_element_count = 0;
    }
    void move_2_front(const_iterator _pos) {
        node_type* _p = _pos._const_cast()._ptr;
        if (_p == &mark) return;
        _p->unhook();
        _p->hook(mark.next);
    }
    void move_2_back(const_iterator _pos) {
        node_type* _p = _pos._const_cast()._ptr;
        if (_p == &mark) return;
        _p->unhook();
        _p->hook(&mark);
    }

    /// ostream
    template <typename _R> friend std::ostream& operator<<(std::ostream& os, const list<_R>& l) {
        os << '[';
        for (auto p = l.cbegin(); p != l.cend(); ++p) {
            os << p;
            if (p + 1 != l.cend()) {
                os << ", ";
            }
        }
        os << ']';
        return os;
    }

protected:
    void _M_init_mark() {
        mark.prev = &mark;
        mark.next = &mark;
    }
    void _M_assign(const self& _l) {
        this->clear();
        for (const_iterator _i = _l.cbegin(); _i != _l.cend(); ++_i) {
            this->push_back(*_i);
        }
    }

private:
    // node_type* head = nullptr; // head->prev = nullptr
    // node_type* tail = nullptr; // tail->next = nullptr
    // head = mark.next;  tail = mark.prev;
    node_type mark;
    size_type m_element_count = 0;
};
};


#endif //! #ifndef _ASP_LIST_HPP_