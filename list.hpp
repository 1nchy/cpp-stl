#ifndef _ASP_LIST_HPP_
#define _ASP_LIST_HPP_

#include <ostream>

#include "list_node.hpp"

namespace asp {
template <typename _Tp> class list;
template <typename _Tp> class list {
public:
    using value_type = typename list_node<_Tp>::value_type;
    using pointer = typename list_node<_Tp>::pointer;
    using reference = typename list_node<_Tp>::reference;
    typedef list_iterator<value_type> iterator;
    typedef list_const_iterator<value_type> const_iterator;
    list() {
        mark.prev = &mark;
        mark.next = &mark;
    }
    virtual ~list() {
        list_node<value_type>* p = mark.next;
        while (p != &mark) {
            auto pnext = p->next;
            delete p;
            p = pnext;
        }
    };

    const value_type& front() const { return mark.next->val(); }
    value_type& front() { return mark.next->val(); }
    const value_type& back() const { return mark.prev->val(); }
    value_type& back() { return mark.prev->val(); }
    const size_type& size() const { return m_element_count; }
    bool empty() const { return size() == 0; }

    /// iterator
    iterator begin() { return iterator(mark.next); }
    const_iterator cbegin() const { return const_iterator(mark.next); }
    iterator end() { return iterator(&mark); }
    const_iterator cend() const { return const_iterator(&mark); }

    void push_front(const value_type& e) { // to head
        list_node<value_type>* p = new list_node<value_type>(e);
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
        list_node<value_type>* p = mark.next;
        p->unhook();
        delete p;
        --m_element_count;
    }
    void push_back(const value_type& e) { // to tail
        list_node<value_type>* p = new list_node<value_type>(e);
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
        list_node<value_type>* p = mark.prev;
        p->unhook();
        delete p;
        --m_element_count;
    }
    iterator insert(const_iterator pos, const value_type& e) {
        list_node<value_type>* p = new list_node<value_type>(e);
        p->hook(pos._const_cast()._ptr);
        ++m_element_count;
        return iterator(p);
    }
    iterator erase(const_iterator pos) {
        iterator _ret = iterator(pos._const_cast()._ptr->next);
        list_node<value_type>* p =pos._const_cast()._ptr;
        p->unhook();
        --m_element_count;
        delete p;
        return _ret;
    }
    iterator erase(const_iterator first, const_iterator last) {
        iterator _ret = iterator(last._const_cast()._ptr);
        list_node<value_type>* p = first._const_cast()._ptr;
        while (p != last._ptr) {
            auto _tmp = p->next;
            p->unhook();
            delete p;
            --m_element_count;
            p = _tmp;
        }
        return _ret;
    }

    /// ostream
    template <typename _R> friend std::ostream& operator<<(std::ostream& os, const list<_R>& l) {
        os << '[';
        for (auto p = l.cbegin(); p != l.cend(); ++p) {
            os << *p;
            if (p + 1 != l.cend()) {
                os << ", ";
            }
        }
        os << ']';
        return os;
    }
private:
    // list_node<value_type>* head = nullptr; // head->prev = nullptr
    // list_node<value_type>* tail = nullptr; // tail->next = nullptr
    // head = mark.next;  tail = mark.prev;
    list_node<value_type> mark;
    size_type m_element_count = 0;
};
};


#endif //! #ifndef _ASP_LIST_HPP_