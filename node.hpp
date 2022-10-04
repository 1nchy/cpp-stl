#ifndef _ASP_NODE_HPP_
#define _ASP_NODE_HPP_

#include <memory>

#include "basic_param.hpp"

namespace asp {
template <typename _Tp> struct node;

template <typename _Tp> struct node {
    // typedef typename _Tp value_type;
    using value_type = _Tp;
    using pointer = _Tp*;
    using reference = _Tp&;
    node(): _pt(std::make_unique<value_type>()) {}
    node(const value_type& t): _pt(std::make_unique<value_type>(t)) {}
    node(const node<value_type>& rhs): _pt(std::make_unique<value_type>(rhs.val())) {}
    node(node<value_type>&& rhs): _pt(rhs._pt) {}
    virtual node& operator=(const node<value_type>& rhs) {
        _pt = std::make_unique<value_type>(rhs.val());
        return *this;
    }
    virtual node& operator=(node<value_type>&& rhs) {
        _pt = std::move(rhs._pt);
        return *this;
    }
    virtual ~node() = default;

    value_type& val() { return *(_pt.get()); }
    const value_type& val() const { return *(_pt.get()); }
    value_type* valptr() { return _pt.get(); }
    const value_type* valptr() const { return _pt.get(); }

    void emplace(const value_type& _e) {
        _pt = std::make_unique<value_type>(_e);
    }
    template <typename... _Args> void emplace(_Args&&... _args) {
        _pt = std::make_unique<value_type>(std::forward<_Args>(_args)...);
    }

    virtual operator bool() const {
        return _pt.get() != nullptr;
    }
    
private:
    std::unique_ptr<value_type> _pt;
};

}; //! namespace asp

#endif