#ifndef _ASP_RANDOM_OBJECT_HPP_
#define _ASP_RANDOM_OBJECT_HPP_

#include "basic_param.hpp"
#include "random.hpp"

#include <string>

#include <cassert>

namespace asp {
/**
 * @brief generate random object
 * @details
 *   int _a = asp::random_object<int>().rand(3, 10);
 *   std::string _s = asp::random_object<std::string>().rand(3, 7);
*/

// basic sample char set
static constexpr char _s_char_set[]
 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";


template <typename _Tp> struct random_object;
struct random_object_base {
    random_object_base() { asp::_S_init_random_seed(); }
};

// #define _IS_UNSIGNED_TYPE(_type) ((_type)-1>0)

template <typename _Tp> struct random_object : public random_object_base {
    typedef _Tp obj_type;
    typedef _Tp bound_type;
    template <typename _U = obj_type> asp::enable_if_t<asp::is_integral<_U>::value, _U> rand(_Tp _l = 0, _Tp _u = 2) const {
        static_assert(std::is_same<_U, _Tp>::value);
        assert(_l <= _u && _l >= 0);
        if (_l == _u) { return obj_type(std::rand()); }
        return obj_type(std::rand() % (_u - _l) + _l);
    };
    template <typename _U = obj_type> asp::enable_if_t<asp::is_floating_point<_U>::value, _U> rand(_Tp _l = 0.0, _Tp _u = 1.0) const {
        static_assert(std::is_same<_U, _Tp>::value);
        assert(_l < _u && _l >= 0);
        obj_type _span = _u - _l;
        return obj_type(_l + (static_cast<_Tp>(std::rand()) / static_cast<_Tp>(RAND_MAX / _span)));
    };
};
template <> struct random_object<char> : public random_object_base {
    typedef char obj_type;
    typedef void bound_type;
    obj_type rand() const {
        return _s_char_set[std::rand() % sizeof(_s_char_set)];
    }
};
template <> struct random_object<std::string> : public random_object_base {
    typedef std::string obj_type;
    typedef size_type bound_type;
    obj_type rand(size_type _l, size_type _u) const {
        assert(_l <= _u);
        size_type _len = random_object<size_type>().rand(_l, _u);
        random_object<char> _ro_char;
        std::string _str;
        _str.reserve(_len);
        for (size_type _i = 0; _i < _len; ++_i) {
            char _c = _ro_char.rand();
            _str.push_back(_c);
        }
        return _str;
    }
};

};
#endif // _ASP_RANDOM_OBJECT_HPP_