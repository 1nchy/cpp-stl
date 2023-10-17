#ifndef _ASP_SHORT_TREE_HPP_
#define _ASP_SHORT_TREE_HPP_

#include <cassert>
#include <vector>

#include "node.hpp"
#include "basic_param.hpp"

namespace asp {

template <typename _Tp> struct short_tree_node;
template <typename _Tp, typename _Alloc> struct short_tree_alloc;
template <typename _Tp> struct short_tree_header;
template <typename _Tp, typename _Alloc = std::allocator<_Tp>> class short_tree;

template <typename _Tp> struct short_tree_node : public node<_Tp> {
    typedef node<_Tp> base;
    typedef short_tree_node<_Tp> self;
    typedef typename base::value_type value_type;

    self* _parent = nullptr;
    self* _left_bro = nullptr;
    self* _right_bro = nullptr;
    self* _first_child = nullptr;
    self* _last_child = nullptr;

    short_tree_node() : base() {}
    short_tree_node(const value_type& _v) : base(_v) {}
    template <typename... _Args> short_tree_node(_Args&&... _args): base(std::forward<_Args>(_args)...) {}
    short_tree_node(const self& _r) = default;
    short_tree_node(self&& _r) = default;
    self& operator=(const self& _r) = default;
    virtual ~short_tree_node() = default;

    void swap(self* _r) { // swap 5 pointer, not the value
        assert(_r != nullptr);
        std::swap(this->_parent, _r->_parent);
        std::swap(this->_left_bro, _r->_left_bro); std::swap(this->_right_bro, _r->_right_bro);
        std::swap(this->_first_child, _r->_first_child); std::swap(this->_last_child, _r->_last_child);
    }
    void unhook() { // reset all pointer
        _parent = nullptr;
        _left_bro = nullptr; _right_bro = nullptr;
        _first_child = nullptr; _last_child = nullptr;
    }
    void copy_value(const self& _r) {
        base::operator=(_r);
    }
};

template <typename _Tp, typename _Alloc> struct short_tree_alloc : public _Alloc {
    typedef short_tree_node<_Tp> node_type;
    typedef typename node_type::value_type value_type;
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

template <typename _Tp> struct short_tree_header {
    typedef short_tree_header<_Tp> self;
    typedef short_tree_node<_Tp> node_type;
    typedef typename node_type::value_type value_type;

    short_tree_node<_Tp> _header;
    size_type _node_count = 0;

    short_tree_header() = default;
    short_tree_header(const self& _r) {
        _header.copy_value(_r);
    }
    void copy_value(const self& _r) {
        _header.copy_value(_r);
    }
    void reset() { _header.unhook(); _node_count = 0; }
};

/**
 * @brief short_tree: decrease the height of the short_tree as possible, by compression, etc.
 * @details
 *   memory model: ←→↑↓↙↖↗↘
 *   ----------------------------------
 *               (_header)   // any node's parent can't be nullptr, %_header->_parent = _root
 *                  ↓↑↓
 *                (_root)
 *         ↙                 ↘
 *      (p1) ←→ (p2) ←→ (p3) ←→ (p4)   // first level children, compress node to here as possible, '1lc' for short
 *            ↙    ↘
 *          (p5) ←→ (p6)
 *         ↙↙
 *        (p7)
 *   -----------------------------------
 *   - add node
 *     append the node at the tail of 1lc,
 *     and update %p4 and %_root.
 *   - del node
 *     for example, if i wanna remove %p2, 
 *     we should find its first 'grand children much later' -- %p7.
 *     so we swap(p2, p7), and update %p5 and original %p7.
 *   - compress node
 *     for example, if i wanna compress %p5,
 *     we need to move {from %p5 to %p5->_parent->last_child} to %p4 (_root->last_child),
 *     so we unhook_pl(p5), and move %p5,
 *     and update %p2, %p5->left_bro (if existed), %p4, %_root.
 *   - merge another short_tree %_r
 *     for convenience, suppose that %_r has the same structure with %this
 *     first, move {from %_r->_root->left_child to %_r->_root->right_child} to the tail of 1lc,
 *     update %p4 and %_root,
 *     second, add %_r->_root to the tail of 1lc,
 *     update.
 *   
*/
template <typename _Tp, typename _Alloc> class short_tree : public short_tree_alloc<_Tp, _Alloc> {
public:
    typedef short_tree<_Tp, _Alloc> self;
    typedef short_tree_alloc<_Tp, _Alloc> base;
    typedef typename base::elt_allocator_type elt_allocator_type;
    typedef typename base::elt_alloc_traits elt_alloc_traits;
    typedef typename base::node_allocator_type node_allocator_type;
    typedef typename base::node_alloc_traits node_alloc_traits;

    typedef typename base::node_type node_type;
    typedef typename node_type::value_type value_type;
 
    template <typename _K, typename _V, typename _Ek, typename _Ev,
     typename _H, typename _A
    > friend class uf_table;
    // > friend class uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>;

private:
    short_tree_header<_Tp> _m_impl;

public:
    short_tree() = default;
    short_tree(const self& _rhs);
    self& operator=(const self& _rhs);
    virtual ~short_tree();

    size_type size() const { return _m_impl._node_count; }
    bool empty() const { return _m_impl._node_count == 0; }
    node_type* add(const value_type& _v);
    void del(node_type* _x);
    node_type* root() const { return _m_impl._header._parent; }
    // return root of %_p, but may compress %_p
    node_type* top(node_type* _x);
    // compress %_p, and make %_p 1st level child node
    void compress(node_type* _x);
    // merge %_r, and make it 1st level child node
    void merge(self& _r);
    void clear();
    void elect(node_type* _e);
    int check() const;
    template <typename _NodeHandler> void traverse(const _NodeHandler& _handler) const { _M_traverse(root(), _handler); };
    static node_type* query (node_type* _x);

    template <typename _T, typename _A> friend
     std::ostream& operator<<(std::ostream& _os, const short_tree<_T, _A>& _s);
protected:
    template <typename _NodeGen> void _M_assign(const self& _r, const _NodeGen& _gen);
    /**
     * @brief clone subtree %_x and its brothers to %_p->children
     * @details generate new nodes named %_n with %_x and its brothers,
     *          and set %_p as the parent of %_n,
     *          clone recursively.
     * @return first %_n
    */
    template <typename _NodeGen> node_type* _M_clone_subtree(const node_type* const _x, node_type* _p, const _NodeGen& _gen);
    void _M_erase_subtree(node_type* _s);

    node_type* _M_hook_root(node_type* _x);

    node_type* _M_first_descendant(node_type* _p) const;
    void _M_append_tail(node_type* _x, node_type* _p = nullptr);
    void _M_append_tail(node_type* _first, node_type* _last, node_type* _p);
    /**
     * @details unhook %_p from its %_parent and %_left_bro, and update them
     * @return %_x->_parent->_last_child
    */
    node_type* _M_unhook_pl(node_type* _x);
    /**
     * @param %_x must be a non-child node
     * @details make %_x->right_bro become its parent's first_child if it was
     * @return %_x->right_bro
    */
    node_type* _M_depose_child(node_type* _x);

    /**
     * @brief swap %_x->val() and %_y->val()
    */
    void _M_swap_node_value(node_type* const _x, node_type* const _y);
    /**
     * @brief swap %_x and %_y, not swap value or change address
    */
    void _M_swap_node(node_type* const _x, node_type* const _y);
    // no close relation between _x and _y
    void _M_swap_normal_node(node_type* const _x, node_type* const _y);
    // _l and _r are brothers, _l == _r->_left_bro && _l->_right_bro == _r
    void _M_swap_bro_node(node_type* const _l, node_type* const _r);
    // _p and _c are in parent-child relation, _p == _c->_parent
    void _M_swap_pc_node(node_type* const _p, node_type* const _c);
    typedef enum {
        __NORMAL__,
        __L_BRO__,
        __R_BRO__,
        __PARENT__,
        __CHILD__,
        __NONE__
    } node_relation_t;
    // %_x is $return of %_y
    node_relation_t _M_relation_of(node_type* const _x, node_type* const _y) const;

    void _M_single_compress(node_type* _x);
    void _M_multi_compress(node_type* _x);
    void _M_add(node_type* _x);
    void _M_del(node_type* _x);
    template <typename _NodeHandler> void _M_traverse(node_type* _x, const _NodeHandler& _handler) const;
    template <typename _Ca> int _M_check(const node_type* const _p, node_type* const _x, const _Ca& _check_assistant) const;
};
/// (de)constructor
template <typename _Tp, typename _Alloc>
short_tree<_Tp, _Alloc>::short_tree(const self& _rhs) {
    _M_assign(_rhs, [this](const node_type* _n) {
        node_type* _p = this->_M_allocate_node(*_n);
        return _p;
    });
    _m_impl._node_count = _rhs.size();
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs == this) return *this;
    clear();
    _M_assign(_rhs, [this](const node_type* _n) {
        node_type* _p = this->_M_allocate_node(*_n);
        return _p;
    });
    _m_impl._node_count = _rhs.size();
};
template <typename _Tp, typename _Alloc>
short_tree<_Tp, _Alloc>::~short_tree() {
    _M_erase_subtree(root());
};

/// protected implementation
template <typename _Tp, typename _Alloc> template <typename _NodeGen> auto
short_tree<_Tp, _Alloc>::_M_assign(const self& _r, const _NodeGen& _gen) -> void {
    _m_impl.reset();
    if (_r.root() == nullptr) return;
    _m_impl._node_count = _r._m_impl._node_count;
    node_type* _n = _M_clone_subtree(_r.root(), &_m_impl._header, _gen);
    _m_impl._header._parent = _n;
};
template <typename _Tp, typename _Alloc> template <typename _NodeGen> auto
short_tree<_Tp, _Alloc>::_M_clone_subtree(const node_type* const _x, node_type* _p, const _NodeGen& _gen)
 -> node_type* {
    assert(_p != nullptr);
    for (const node_type* _i = _x; _i != nullptr; _i = _i->_right_bro) {
        node_type* const _n = _gen(_x);
        _n->_parent = _p;
        if (_x->_left_bro == nullptr) _p->_first_child = _n;
        if (_x->_right_bro == nullptr) _p->_last_child = _n;
        _M_clone_subtree(_x->_first_child, _n, _gen);
    }
    return _p->_first_child;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_erase_subtree(node_type* _s) -> void {
    if (_s == nullptr) return;
    for (node_type* _i = _s->_first_child; _i != nullptr; _i = _i->_right_bro) {
        _M_erase_subtree(_i);
    }
    this->_M_deallocate_node(_s);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_hook_root(node_type* _x) -> node_type* {
    // please make sure %_m_impl._header._parent = nullptr;
    node_type* const _ret = _m_impl._header._parent;
    _m_impl._header._first_child = _x;
    _m_impl._header._last_child = _x;
    _m_impl._header._parent = _x;
    _x->_parent = &_m_impl._header;
    return _ret;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_first_descendant(node_type* _p) const -> node_type* {
    if (_p == nullptr) return nullptr;
    while (_p->_first_child != nullptr) {
        _p = _p->_first_child;
    }
    return _p;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_append_tail(node_type* _x, node_type* _p) -> void {
    // todo update root()->_first_child
    if (_p == nullptr) _p = root();
    assert(_p != nullptr);
    if (_x == nullptr) return;
    // _x->unhook();
    node_type* const _tail = _p->_last_child;
    _x->_left_bro = _tail;
    if (_tail != nullptr) _tail->_right_bro = _x;
    if (_p->_first_child == nullptr) _p->_first_child = _x;
    _p->_last_child = _x;
    _x->_parent = _p;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_append_tail(node_type* _f, node_type* _l, node_type* _p) -> void {
    // todo update root()->_first_child
    if (_p == nullptr) _p = root();
    assert(_p != nullptr);
    if (_f == nullptr || _l == nullptr) return;
    node_type* const _tail = _p->_last_child;
    _f->_left_bro = _tail;
    if (_tail != nullptr) _tail->_right_bro = _f;
    if (_p->_first_child == nullptr) _p->_first_child = _f;
    _p->_last_child = _l;
    for (node_type* _i = _f; _i != nullptr; _i = _i->_right_bro) {
        _i->_parent = _p;
        if (_i == _l) break;
    }
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_unhook_pl(node_type* _x) -> node_type* {
    if (_x == nullptr || _x == root()) return nullptr;
    // _x->_parent != nullptr;
    node_type* const _p = _x->_parent;
    assert(_p != nullptr);
    node_type* const _tail = _p->_last_child;
    _p->_last_child = _x->_left_bro;
    if (_x->_left_bro == nullptr) _p->_first_child = nullptr;
    else _x->_left_bro->_right_bro = nullptr;
    return _tail;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_depose_child(node_type* _x) -> node_type* {
    if (_x == nullptr) return nullptr;
    assert(_x->_first_child == nullptr && _x->_last_child == nullptr);
    node_type* const _p = _x->_parent; // _p != nullptr
    if (_p->_first_child == _x) _p->_first_child = _x->_right_bro;
    else _x->_left_bro->_right_bro = _x->_right_bro;
    if (_p->_last_child == _x) _p->_last_child = nullptr;
    else _x->_right_bro->_left_bro = _x->_left_bro;
    if (_x == root()) _p->_parent = nullptr;
    return _x->_right_bro;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_swap_node_value(node_type* const _x, node_type* const _y) -> void {
    if (_x == nullptr || _y == nullptr || _x == _y) return;
    assert(_x != &_m_impl._header && _y != &_m_impl._header);
    std::swap(_x->val(), _y->val());
}
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_swap_node(node_type* const _x, node_type* const _y) -> void {
    // todo : when _x->_parent == _y
    if (_x == nullptr || _y == nullptr || _x == _y) return;
    assert(_x != &_m_impl._header && _y != &_m_impl._header);
    auto _relation = _M_relation_of(_x, _y);
    assert(_relation != node_relation_t::__NONE__);
    switch (_relation) {
    case node_relation_t::__NORMAL__: { _M_swap_normal_node(_x, _y); }; break;
    case node_relation_t::__L_BRO__: { _M_swap_bro_node(_x, _y); }; break;
    case node_relation_t::__R_BRO__: { _M_swap_bro_node(_y, _x); }; break;
    case node_relation_t::__PARENT__: { _M_swap_pc_node(_x, _y); }; break;
    case node_relation_t::__CHILD__: { _M_swap_pc_node(_y, _x); }; break;
    }
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_swap_normal_node(node_type* const _x, node_type* const _y) -> void {
    // parent relation
    {
    node_type* const _xp = _x->_parent;
    node_type* const _yp = _y->_parent;
    bool _xp_fc = _xp->_first_child == _x;
    bool _xp_lc = _xp->_last_child == _x;
    bool _yp_fc = _yp->_first_child == _y;
    bool _yp_lc = _yp->_last_child == _y;
    if (_xp_fc) _xp->_first_child = _y;
    if (_xp_lc) _xp->_last_child = _y;
    if (_yp_fc) _yp->_first_child = _x;
    if (_yp_lc) _yp->_last_child = _x;
    }
    // brothers relation
    {
    node_type* const _x_lb = _x->_left_bro;
    node_type* const _x_rb = _x->_right_bro;
    node_type* const _y_lb = _y->_left_bro;
    node_type* const _y_rb = _y->_right_bro;
    if (_x_lb != nullptr) _x_lb->_right_bro = _y;
    if (_x_rb != nullptr) _x_rb->_left_bro = _y;
    if (_y_lb != nullptr) _y_lb->_right_bro = _x;
    if (_y_rb != nullptr) _y_rb->_left_bro = _x;
    }
    // children relation
    {
    node_type* const _x_fc = _x->_first_child;
    node_type* const _y_fc = _y->_first_child;
    for (node_type* _i = _x_fc; _i != nullptr; _i = _i->_right_bro) {
        _i->_parent = _y;
    }
    for (node_type* _i = _y_fc; _i != nullptr; _i = _i->_right_bro) {
        _i->_parent = _x;
    }
    }
    // _header
    {
    if (_x == root()) _m_impl._header._parent = _y;
    else if (_y == root()) _m_impl._header._parent = _x;
    }
    // self pointer
    _x->swap(_y);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_swap_bro_node(node_type* const _l, node_type* const _r) -> void {
    assert(_l->_parent == _r->_parent && _l->_right_bro == _r && _r->_left_bro == _l);
    // parent relation
    {
    node_type* const _p = _l->_parent;
    bool _p_fc = _p->_first_child == _l;
    bool _p_lc = _p->_last_child == _r;
    if (_p_fc) _p->_first_child = _r;
    if (_p_lc) _p->_last_child = _l;
    }
    // brothers relation
    {
    node_type* const _l_lb = _l->_left_bro;
    node_type* const _r_rb = _r->_right_bro;
    if (_l_lb != nullptr) _l_lb->_right_bro = _r;
    if (_r_rb != nullptr) _r_rb->_left_bro = _l;
    }
    // children relation
    {
    node_type* const _l_fc = _l->_first_child;
    node_type* const _r_fc = _r->_first_child;
    for (node_type* _i = _l_fc; _i != nullptr; _i = _i->_right_bro) {
        _i->_parent = _r;
    }
    for (node_type* _i = _r_fc; _i != nullptr; _i = _i->_right_bro) {
        _i->_parent = _l;
    }
    }
    // _header
    {}
    // self pointer
    _l->swap(_r);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_swap_pc_node(node_type* const _p, node_type* const _c) -> void {
    assert(_p == _c->_parent);
    bool _p_fc = _p->_first_child == _c;
    bool _p_lc = _p->_last_child == _c;
    // parent relation
    {
    node_type* const _pp = _p->_parent;
    bool _pp_fc = _pp->_first_child == _p;
    bool _pp_lc = _pp->_last_child == _p;
    if (_pp_fc) _pp->_first_child = _c;
    if (_pp_lc) _pp->_last_child = _c;
    }
    // brothers relation
    {
    node_type* const _p_lb = _p->_left_bro;
    node_type* const _p_rb = _p->_right_bro;
    node_type* const _c_lb = _c->_left_bro;
    node_type* const _c_rb = _c->_right_bro;
    if (_p_lb != nullptr) _p_lb->_right_bro = _c;
    if (_p_rb != nullptr) _p_rb->_left_bro = _c;
    if (_c_lb != nullptr) _c_lb->_right_bro = _p;
    if (_c_rb != nullptr) _c_rb->_left_bro = _p;
    }
    // children relation
    {
    node_type* const _p_fc = _p->_first_child;
    node_type* const _c_fc = _c->_first_child;
    for (node_type* _i = _p_fc; _i != nullptr; _i = _i->_right_bro) {
        if (_i != _c) _i->_parent = _c;
    }
    for (node_type* _i = _c_fc; _i != nullptr; _i = _i->_right_bro) {
        _i->_parent = _p;
    }
    }
    // _header
    {
    if (_p == root()) _m_impl._header._parent = _c;
    }
    // self pointer
    _p->swap(_c);
    if (_p_fc) _c->_first_child = _p;
    if (_p_lc) _c->_last_child = _p;
    _p->_parent = _c;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_relation_of(node_type* const _x, node_type* const _y) const
-> node_relation_t {
    if (_x == nullptr || _x == &_m_impl._header || _y == nullptr || _y == &_m_impl._header)
        return node_relation_t::__NONE__;
    if (_x->_left_bro == _y && _y->_right_bro == _x) return node_relation_t::__R_BRO__;
    if (_x->_right_bro == _y && _y->_left_bro == _x) return node_relation_t::__L_BRO__;
    if (_x->_parent == _y) return node_relation_t::__CHILD__;
    if (_y->_parent == _x) return node_relation_t::__PARENT__;
    return node_relation_t::__NORMAL__;
};

template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_single_compress(node_type* _x) -> void {
    if (_x == nullptr || _x == root()) return;
    node_type* const _d = _M_first_descendant(_x);
    // _M_swap_node(_x, _d);
    // _M_depose_child(_x);
    // _M_append_tail(_x);
    _M_swap_node_value(_x, _d);
    _M_depose_child(_d);
    _M_append_tail(_x);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_multi_compress(node_type* _x) -> void {
    if (_x == nullptr || _x == root()) return;
    node_type* const _tail = _M_unhook_pl(_x);
    _M_append_tail(_x, _tail, root());
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_add(node_type* _x) -> void {
    if (root() != nullptr) {
        _M_append_tail(_x);
    }
    else {
        _M_hook_root(_x);
    }
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::_M_del(node_type* _x) -> void {
    node_type* const _d = _M_first_descendant(_x);
    // _M_swap_node(_x, _d);
    // _M_depose_child(_x);
    // this->_M_deallocate_node(_x);
    _M_swap_node_value(_x, _d);
    _M_depose_child(_d);
    this->_M_deallocate_node(_d);
};

/// public implementation
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::add(const value_type& _v) -> node_type* {
    node_type* _n = this->_M_allocate_node(_v);
    _M_add(_n);
    ++_m_impl._node_count;
    return _n;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::del(node_type* _x) -> void {
    _M_del(_x);
    --_m_impl._node_count;
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::top(node_type* _x) -> node_type* {
    compress(_x);
    return root();
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::compress(node_type* _x) -> void {
    if (_x == nullptr || _x == root()) return;
    _M_single_compress(_x);
    // _M_multi_compress(_x);
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::merge(self& _r) -> void {
    if (_r.root() == nullptr) return;
    if (root() == nullptr) {
        _M_hook_root(_r.root());
    }
    else {
        node_type* const _p = _r.root();
        // _M_append_tail(_p->_first_child, _p->_last_child, root());
        _M_append_tail(_p);
    }
    _m_impl._node_count += _r.size();
    _r._m_impl.reset();
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::clear() -> void {
    _M_erase_subtree(root());
    _m_impl.reset();
};
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::elect(node_type* _e) -> void {
    if (_e == root()) nullptr;
    _M_swap_node_value(_e, root());
};
template <typename _Tp, typename _Alloc> template <typename _NodeHandler> auto
short_tree<_Tp, _Alloc>::_M_traverse(node_type* _x, const _NodeHandler& _handler) const -> void {
    if (_x == nullptr) return;
    for (node_type* _i = _x; _i != nullptr; _i = _i->_right_bro) {
        _handler(_i);
        _M_traverse(_i->_first_child, _handler);
    }
};

/// static implementation
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::query(node_type* _x) -> node_type* {
    if (_x == nullptr) return nullptr;
    while (_x->_parent->_parent != _x) {
        _x = _x->_parent;
    }
    return _x;
};

/// check implementation
/**
 * @brief check the short_tree
 * @returns 0 : normal
 *   1 : the parent of node's children isn't itself
 *   2 : node has some other children which not in fl range
 *       (children link doesn't start with parent's first_child and end up with parent's last_child)
 *   3 : the number of nodes inequal %size()
 *   4 : error in %_header
 *   5 : error in %_header when empty
 *   6 : error in brothers' link
*/
template <typename _Tp, typename _Alloc> auto
short_tree<_Tp, _Alloc>::check() const -> int {
    std::vector<const node_type*> _vn;
    // _header check
    node_type* _r = root();
    if (_r == nullptr) {
        if (_m_impl._header._left_bro != nullptr || _m_impl._header._right_bro != nullptr ||
        _m_impl._header._first_child != nullptr || _m_impl._header._last_child != nullptr) {
            return 5;
        }
        return 0;
    }
    if (_m_impl._header._first_child != _r || _m_impl._header._last_child != _r || _r->_parent != &_m_impl._header) {
        return 4;
    }
    // recursively check
    int _result = _M_check(&_m_impl._header, root(), [&](const node_type* _x) {
        _vn.push_back(_x);
    });
    if (_result != 0) return _result;
    if (_vn.size() != size()) return 3;
    return 0;
};
template <typename _Tp, typename _Alloc> template <typename _Ca> auto
short_tree<_Tp, _Alloc>::_M_check(const node_type* const _p, node_type* const _x, const _Ca& _check_assistant) const -> int {
    if (_x->_parent != _p) return 1;
    _check_assistant(_x);
    for (node_type* _i = _x->_first_child; _i != nullptr; _i = _i->_right_bro) {
        if (_i == _x->_first_child && _i->_left_bro != nullptr) {
            return 2;
        }
        if (_i == _x->_last_child && _i->_right_bro != nullptr) {
            return 2;
        }
        if (_i->_right_bro != nullptr && _i->_right_bro->_left_bro != _i) {
            return 6;
        }
        int _result = _M_check(_x, _i, _check_assistant);
        if (_result != 0) return _result;
    }
    return 0;
};

/// output implement
template <typename _T, typename _A> auto
operator<<(std::ostream& _os, const short_tree<_T, _A>& _s) -> std::ostream& {
    _os << '[';
    int _cnt = 0;
    _s.traverse([&_os, &_cnt](const auto* _p) {
        if (_cnt != 0) _os << ", ";
        _os << _p->val();
        ++_cnt;
    });
    return _os << ']';
};

};

#endif // _ASP_SHORT_TREE_HPP_