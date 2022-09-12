#ifndef _UNION_FIND_SET_H_
#define _UNION_FIND_SET_H_

#include <unordered_map>
#include <utility>
#include <vector>

template <typename _Tp> class uf_set;

template <typename _Tp> class uf_set {
public:
    using value_type = _Tp;
    using size_type = uint32_t;
    using hash_type = size_type;
protected:
    const constexpr static hash_type MAX_SIZE_VALUE = UINT32_MAX;
    const constexpr static hash_type MAX_HASH_VALUE = UINT32_MAX;

public:
    explicit uf_set() = default;
    explicit uf_set(size_type n);
    virtual ~uf_set() = default;

public:
    bool check(const value_type& a, const value_type& b) const;
    template <typename... Args> bool check(const value_type& e0, const value_type& e1, const Args&... en) const;
    bool insert(const value_type& e);
    bool insert(const value_type& p, const value_type& e);
    template <typename... Args> bool insert(const value_type& p, const value_type& e0, const Args&... en);
    bool erase(const value_type& e);
    bool erase_all(const value_type& p);

    void merge(const value_type& a, const value_type& b);
    size_type count() const;

protected:
    hash_type new_set_id() const;
    hash_type status(const value_type& e);

private:
    size_type m_set_count = 0;
    std::unordered_map<value_type, hash_type> m_elements; // 记录元素和它唯一的下标
    std::vector<std::pair<hash_type, hash_type>> m_table; // 元素下标互指表
};

#endif