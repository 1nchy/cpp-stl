#ifndef _ASP_HASH_TABLE_POLICY_HPP_
#define _ASP_HASH_TABLE_POLICY_HPP_

#include <cmath>

#include "node.hpp"

namespace asp {

struct rehash_policy;
template<typename _Alloc> struct hash_table_alloc;

extern const unsigned long _prime_list[] = {
    3ul,          7ul,          13ul,
    29ul,         53ul,         97ul,         193ul,       389ul,
    769ul,        1543ul,       3079ul,       6151ul,      12289ul,
    24593ul,      49157ul,      98317ul,      196613ul,    393241ul,
    786433ul,     1572869ul,    3145739ul,    6291469ul,   12582917ul,
    25165843ul,   50331653ul,   100663319ul,  201326611ul, 402653189ul,
    805306457ul,  1610612741ul, 3221225473ul, 4294967291ul
};

struct rehash_policy {
    typedef size_type _State;

    rehash_policy(float _z = 1.0) : _max_load_factor(_z), _next_resize(0) {}

    float max_load_factor() const { return _max_load_factor; }
    _State state() const { return _next_resize; }
    void reset(_State _s = 0) { _next_resize = _s; }

    /**
     * @param %_n = current number of buckets
     * @return the next prime number that \ge %_n
    */
    size_type next_bkt(size_type _n) const;
    /**
     * @param %_n = the number of elements
     * @return the least number of buckets, which's able to contain %_n elements.
    */
    size_type bkt_for_elements(size_type _n) const;
    /**
     * @param %_n_bkt = the number of buckets in @hash_table;
     *        %_n_elt = the number of elements in @hash_table;
     *        %_n_ins = the number of elements that need to insert.
     * @return if rehash is needed，the return (true, new_bucket_count); else return (false, 0)
    */
    std::pair<bool, size_type> need_rehash(size_type _n_bkt, size_type _n_elt, size_type _n_ins) const;

    enum { _s_primes = sizeof(_prime_list) / sizeof(_prime_list[0]) };

    static const size_type _s_growth_factor = 2;
    // 负载因子，衡量桶的负载程度
    float _max_load_factor;  // = _n_elt / _n_bkt
    // resize 后，能保存元素个数的最佳上限
    mutable size_type _next_resize; // = _n_bkt * _max_load_factor
};

template<typename _Alloc> struct hash_table_alloc : public _Alloc {
    typedef _Alloc node_allocator_type;
    typedef node_type _Alloc::value_type;
};

size_type rehash_policy::next_bkt(size_type _n) const {
    const unsigned long *_p = std::lower_bound(_prime_list, _prime_list + _s_primes - 1, _n);
    _next_resize = static_cast<size_type>(std::ceil(*_p * _max_load_factor));
    return *_p;
}

size_type rehash_policy::bkt_for_elements(size_type _n) const {
    return std::ceil(_n / (long double)_max_load_factor);
}

std::pair<bool, size_type> rehash_policy::need_rehash(size_type _n_bkt, size_type _n_elt, size_type _n_ins) const {
    if (_n_elt + _n_ins > _next_resize) {
        float _min_bkts = ((float(_n_ins) + float(_n_elt)) / _max_load_factor);
        if (_min_bkts > _n_bkt) {
            _min_bkts = std::max(_min_bkts, float(_s_growth_factor * _n_bkt));
            return std::make_pair(true, next_bkt(static_cast<size_type>(std::ceil(_min_bkts))));
        }
        else {
            _next_resize = static_cast<size_type>(std::ceil(_n_bkt * _max_load_factor));
            return std::make_pair(false, 0);
        }
    }
    else {
        return std::make_pair(false, 0);
    }
}

};

#endif  // _ASP_HASH_TABLE_POLICY_HPP_