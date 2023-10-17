#ifndef _ASP_UNION_FIND_TABLE_HPP_
#define _ASP_UNION_FIND_TABLE_HPP_

#include <cassert>
#include <unordered_map>

#include "associative_container_aux.hpp"
#include "hash_table.hpp"
#include "short_tree.hpp"

namespace asp {
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash = std::hash<_Key>, typename _Alloc = std::allocator<_Value>> struct uf_table;
template <typename _Value, typename _Alloc> struct uf_table_alloc;

template <typename _Tp> using uf_node = short_tree_node<_Tp>;
struct _select_node_value;

namespace {
template <typename _ExtOp> struct _select_ufnode_action {
    template <typename _Tp> auto operator()(uf_node<_Tp>* _x) const {
        // return _ExtOp()(_select_ufnode()(*_x));
        return _ExtOp()(_x->val());
    }
    template <typename _Tp> auto operator()(const uf_node<_Tp>* _x) const {
        // return _ExtOp()(_select_ufnode()(*_x));
        return _ExtOp()(_x->val());
    }
};
};

template <typename _Value, typename _Alloc> struct uf_table_alloc : public _Alloc {
    typedef short_tree<_Value> tree_t;
    typedef _Alloc elt_allocator_type;
    typedef std::allocator_traits<elt_allocator_type> elt_alloc_traits;
    typedef typename elt_alloc_traits::template rebind_alloc<tree_t> tree_allocator_type;
    typedef std::allocator_traits<tree_allocator_type> tree_alloc_traits;

    elt_allocator_type& _M_get_elt_allocator() { return *static_cast<elt_allocator_type*>(this); }
    const elt_allocator_type& _M_get_elt_allocator() const { return *static_cast<const elt_allocator_type*>(this); }
    tree_allocator_type _M_get_tree_allocator() const { return tree_allocator_type(_M_get_elt_allocator()); }

    tree_t* _M_allocate_tree() {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        auto _ptr = tree_alloc_traits::allocate(_tree_alloc, 1);
        tree_t* _p = std::addressof(*_ptr);
        ::new(_p) tree_t();
        return _p;
    }
    tree_t* _M_allocate_tree(const tree_t& _x) {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        auto _ptr = tree_alloc_traits::allocate(_tree_alloc, 1);
        tree_t* _p = std::addressof(*_ptr);
        tree_alloc_traits::construct(_tree_alloc, _p, _x);
        return _p;
    }
    template <typename... _Args> tree_t* _M_allocate_tree(_Args&&... _args) {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        auto _ptr = tree_alloc_traits::allocate(_tree_alloc, 1);
        tree_t* _p = std::addressof(*_ptr);
        tree_alloc_traits::construct(_tree_alloc, _p, std::forward<_Args>(_args)...);
        return _p;
    }
    void _M_deallocate_tree(tree_t* const _p) {
        tree_allocator_type _tree_alloc = _M_get_tree_allocator();
        tree_alloc_traits::destroy(_tree_alloc, _p);
        tree_alloc_traits::deallocate(_tree_alloc, _p, 1);
    }
};

/**
 * @brief uf_table (a short tree)
 * @implements implemented by a hash_set
 * @details
 *   MEMORY:
 *   hash_table _h: store the node pointers of the short_tree
 *   hash_table _sth: store <hash_code, short_tree*>
 * 
 *   API:
 *   add(e): create a uf_table with e as root, or only add e
 *   del(e): delete a from uf_table
 *     if e is root, select another element as root
 *       
 *   there's a root element in each bucket
*/

template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc>
struct uf_table : public uf_table_alloc<_Value, _Alloc> {
    typedef uf_node<_Value> node_t;
    typedef hash_table<_Key, uf_node<_Value>*, _select_ufnode_action<_ExtKey>, true, _select_ufnode_action<_ExtValue>, _Hash, _Alloc> table_t;
    typedef uf_table_alloc<_Value, _Alloc> base;
    typedef typename base::tree_t tree_t;
    typedef uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc> self;

    typedef typename table_t::hash_code hash_code;
    typedef typename table_t::key_type key_type;
    typedef _Value value_type;
    typedef asso_container::type_traits<_Value, true> _ContainerTypeTraits;
    typedef typename _ContainerTypeTraits::mapped_type mapped_type;

    typedef hash_table<hash_code, std::pair<hash_code, tree_t*>, _select_0x, true, _select_1x, _Hash, _Alloc> sttable_t;

    table_t _h; // <key, uf_node*>
    sttable_t _sth; // <hash_code, short_tree*>

public:
    typedef typename table_t::iterator iterator;
    typedef typename table_t::const_iterator const_iterator;

    uf_table() = default;
    uf_table(const self& _rhs);
    self& operator=(const self& _rhs);
    virtual ~uf_table() {}

    bool existed(const key_type& _k) const { return _h.count(_k) != 0; }
    tree_t* delegate(const key_type& _k) const;
    tree_t* delegate(node_t* const _n) const;
    std::pair<bool, hash_code> delegate_id(const key_type& _k) const;
    std::pair<bool, hash_code> delegate_id(node_t* const _n) const;
    std::pair<bool, hash_code> delegate_id(const tree_t& _t) const;
    size_type classification() const { return _sth.size(); }
    size_type size() const { return _h.size(); }
    bool sibling(const key_type& _x, const key_type& _y);
    bool add(const value_type& _v);
    bool add(const key_type& _k, const value_type& _v);
    bool del(const key_type& _k);
    bool merge(const key_type& _x, const key_type& _y);
    bool elect(const key_type& _k);
    bool empty() const { return _h.empty(); }
    void clear();

// check function
    int check() const;
    void demo(std::istream& _is = std::cin, std::ostream& _os = std::cout);

protected:
    template <typename _NodeGen> void _M_assign(const self& _rhs, const _NodeGen& _gen);
    /**
     * @brief update tree information in %_sth
     * @details it's recommended that storing the hash_code of a tree before its any operation.
    */
    void _M_update_tree_info(hash_code _oc, tree_t& _t);
};

template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc>
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::uf_table(const self& _rhs) {
/**
 * assign each short_tree in %_sth to this, and construct %_h
*/
    _M_assign(_rhs, [this](const tree_t* _t) {
        tree_t* _nt = this->_M_allocate_tree();
        _nt->_M_assign(*_t, [this, _nt](const typename tree_t::node_type* _n) {
            typename tree_t::node_type* _p = _nt->_M_allocate_node(*_n);
            this->_h.insert(_p);
            return _p;
        });
        hash_code _c = delegate_id(*_nt).second;
        this->_sth.insert(std::make_pair(_c, _nt));
        return _nt;
    });
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::operator=(const self& _rhs) -> self& {
    if (&_rhs == this) return *this;
    clear();
    _M_assign(_rhs, [this](const tree_t* _t) {
        tree_t* _nt = this->_M_allocate_tree(*_t);
        _nt->_M_assign(*_t, [this, _nt](const typename tree_t::node_type* _n) {
            typename tree_t::node_type* _p = _nt->_M_allocate_node(*_n);
            this->_h.insert(_p);
            return _p;
        });
        hash_code _c = delegate_id(*_nt).second;
        this->_sth.insert(std::make_pair(_c, _nt));
        return _nt;
    });
    return *this;
};

template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::delegate(const key_type& _k) const -> tree_t* {
    if (!existed(_k)) return nullptr;
    return _sth.find(delegate_id(_k).second)->second;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::delegate(node_t* const _n) const -> tree_t* {
    if (_n == nullptr) return nullptr;
    hash_code _c = delegate_id(_n).second;
    return _sth.find(_c)->second;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::delegate_id(const key_type& _k) const -> std::pair<bool, hash_code> {
    if (!existed(_k)) return std::make_pair(false, hash_code());
    node_t* _n = *(_h.find(_k));
    node_t* const _top = tree_t::query(_n);
    return std::make_pair(true, _h._M_hash_code(_h._extract_key(_top)));
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::delegate_id(node_t* const _n) const -> std::pair<bool, hash_code> {
    if (_n == nullptr) return std::make_pair(false, hash_code());
    node_t* const _top = tree_t::query(_n);
    return std::make_pair(true, _h._M_hash_code(_h._extract_key(_top)));
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::delegate_id(const tree_t& _t) const -> std::pair<bool, hash_code> {
    if (_t.root() == nullptr) return std::make_pair(false, hash_code());
    return std::make_pair(true, _h._M_hash_code(_h._extract_key(_t.root())));
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::sibling(const key_type& _x, const key_type& _y) -> bool {
    if (!existed(_x) || !existed(_y)) return false;
    return delegate_id(_x).second == delegate_id(_y).second;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::add(const value_type& _v) -> bool {
    if (existed(_ExtKey()(_v))) return false;
    tree_t* const _st = this->_M_allocate_tree();
    node_t* _n = _st->add(_v);
    hash_code _c = delegate_id(*_st).second;
    _sth.insert(std::make_pair(_c, _st));
    _h.insert(_n);
    return true;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::add(const key_type& _k, const value_type& _v) -> bool {
    if (!existed(_k) || existed(_ExtKey()(_v))) return false;
    tree_t& _t = *delegate(_k);
    node_t* const _n = _t.add(_v);
    _h.insert(_n);
    return true;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::del(const key_type& _k) -> bool {
    if (!existed(_k)) return false;
    node_t* _n = *(_h.find(_k));
    _h.erase(_k);
    tree_t& _t = *delegate(_n);
    hash_code _oc = delegate_id(_t).second;
    _t.del(_n);
    if (!_t.empty()) _h.insert(_t.root());
    _M_update_tree_info(_oc, _t);
    return true;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::merge(const key_type& _x, const key_type& _y) -> bool {
    if (!existed(_x) || !existed(_y)) return false;
    tree_t& _xt = *delegate(_x);
    tree_t& _yt = *delegate(_y);
    hash_code _xtc = delegate_id(_xt).second;
    hash_code _ytc = delegate_id(_yt).second;
    _xt.merge(_yt);
    _M_update_tree_info(_xtc, _xt);
    _M_update_tree_info(_ytc, _yt);
    return true;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::elect(const key_type& _k) -> bool {
    if (!existed(_k)) return false;
    node_t* _n = *(_h.find(_k));
    tree_t& _t = *delegate(_n);
    hash_code _oc = delegate_id(_t).second;
    _h.erase(_k); _h.erase(_ExtKey()(_t.root()->val()));
    _t.elect(_n);
    _h.insert(_n); _h.insert(_t.root());
    _M_update_tree_info(_oc, _t);
    return true;
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::clear() -> void {
    _h.clear();
    for (auto _i = _sth.begin(); _i != _sth.end(); ++_i) {
        (_i->second)->clear();
    }
    _sth.clear();
};

/// protected implementation
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc>
template <typename _NodeGen> auto uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>
::_M_assign(const self& _rhs, const _NodeGen& _gen) -> void {
    for (auto _i = _rhs._sth.cbegin(); _i != _rhs._sth.cend(); ++_i) {
        _gen(_i->second);
    }
};
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::_M_update_tree_info(hash_code _oc, tree_t& _t) -> void {
    if (_t.empty()) {
        this->_M_deallocate_tree(_sth.find(_oc)->second);
        _sth.erase(_oc);
        return;
    }
    hash_code _c = delegate_id(_t).second;
    if (_c != _oc) {
        _sth.erase(_oc);
        _sth.insert(std::make_pair(_c, &_t));
    }
};

/// check implementation
/**
 * @brief check the uf_table
 * @returns 0 : normal
 *   1 : null tree pointer or empty tree in %_sth
 *   2 : null node in %_h
 *   3 : 
 *   4 : 
 *   100+ : error in short tree inside
*/
template <typename _Key, typename _Value, typename _ExtKey, typename _ExtValue, typename _Hash, typename _Alloc> auto
uf_table<_Key, _Value, _ExtKey, _ExtValue, _Hash, _Alloc>::check() const -> int {
    for (auto _i = _sth.cbegin(); _i != _sth.cend(); ++_i) {
        if (_i->second == nullptr || (_i->second)->empty()) return 1;
        int _tmp = _i->second->check();
        if (_tmp != 0) return 100 + _tmp;
    }
    for (auto _i = _h.cbegin(); _i != _h.cend(); ++_i) {
        if (*_i == nullptr) return 2;
    }
    return 0;
};

};

#endif // _ASP_UNION_FIND_TABLE_HPP_