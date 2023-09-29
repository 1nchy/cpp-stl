#ifndef _ASP_NODE_HPP_
#define _ASP_NODE_HPP_

#include <memory>

#include "basic_param.hpp"

namespace asp {
template <typename _Tp> struct node;

template <typename _Tp> struct node {
    // typedef typename _Tp value_type;
    typedef node<_Tp> self;
    using value_type = _Tp;
    using pointer = _Tp*;
    using reference = _Tp&;
    // node(): _pt(std::make_unique<value_type>()) {}
    node() {}
    template <typename... _Args> node(_Args&&... _args): _v(std::forward<_Args>(_args)...) {}
    node(const value_type& _x): _v(_x) {}
    node(const node<value_type>& _rhs): _v(_rhs._v) {}
    node(node<value_type>&& _rhs): _v(std::move(_rhs._v)) {}
    node& operator=(const node<value_type>& _rhs) {
        if (&_rhs == this) return *this;
        // _pt = std::make_unique<value_type>(rhs.val());
        _v = _rhs._v;
        return *this;
    }
    node& operator=(node<value_type>&& _rhs) {
        // _pt = std::move(rhs._pt);
        _v = std::move(_rhs._v);
        return *this;
    }
    virtual ~node() = default;

    // value_type& val() { return *(_pt.get()); }
    // const value_type& val() const { return *(_pt.get()); }
    // value_type* valptr() { return _pt.get(); }
    // const value_type* valptr() const { return _pt.get(); }
    value_type& val() { return _v; }
    const value_type& val() const { return _v; }
    value_type* valptr() { return &_v; }
    const value_type* valptr() const { return &_v; }

    // void emplace(const value_type& _e) {
    //     _pt = std::make_unique<value_type>(_e);
    // }
    // template <typename... _Args> void emplace(_Args&&... _args) {
    //     _pt = std::make_unique<value_type>(std::forward<_Args>(_args)...);
    // }

    // virtual operator bool() const {
    //     return _pt.get() != nullptr;
    // }
    friend bool operator==(const self& _x, const self& _y) {
        return _x.val() == _y.val();
    }
    friend bool operator!=(const self& _x, const self& _y) {
        return _x.val() != _y.val();
    }

private:
    // std::unique_ptr<value_type> _pt;
    value_type _v;
};

}; //! namespace asp

#endif