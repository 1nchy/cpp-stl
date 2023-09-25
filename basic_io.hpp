#ifndef _ASP_BASIC_IO_HPP_
#define _ASP_BASIC_IO_HPP_

#include "basic_param.hpp"

#include <string>
#include <tuple>
#include <ostream>
#include <istream>
#include <sstream>

namespace asp {
// std::ostream& operator<<(std::ostream& os, const std::string& _s) {
//     std::operator<<(os, '\"'+ _s + '\"');
//     return os;
// };
namespace obj_string {
template <typename _Tp> std::string _M_obj_2_string(const _Tp& _t) {
    std::stringstream ss;
    ss << _t;
    return ss.str();
}
template <> std::string _M_obj_2_string<std::string>(const std::string& _t) {
    return '\"' + _t + '\"';
}
};
namespace __details__ {
template <typename _Tuple, size_type _N> struct tuple_print;
template <typename _Tuple, size_type _N> struct tuple_print {
    static void print(const _Tuple& t, std::ostream& os) {
        tuple_print<_Tuple, _N-1>::print(t, os);
        os << ", " << std::get<_N-1>(t); 
    }
};
template<typename _Tuple> struct tuple_print<_Tuple, 1> {
    static void print(const _Tuple& t, std::ostream& os) {
        os << '{' << std::get<0>(t); 
    }
};
};

template <typename... _Args> auto operator<<(std::ostream& os, const std::tuple<_Args...>& _t)
-> std::ostream& {
    __details__::tuple_print<decltype(_t), sizeof...(_Args)>::print(_t, os);
    return os << '}';
};

template <typename _Head, typename _Tail> auto operator<<(std::ostream& os, const std::pair<_Head, _Tail>& _t)
-> std::ostream& {
    return os << '{' << obj_string::_M_obj_2_string(_t.first) << ", " << obj_string::_M_obj_2_string(_t.second) << '}';
}

template <typename _R> std::ostream& operator<<(std::ostream& os, const std::vector<_R>& v) {
    os << '[';
    for (auto p = v.cbegin(); p != v.cend(); ++p) {
        os << *p;
        if (p + 1 != v.cend()) {
            os << ", ";
        }
    }
    os << ']';
    return os;
}


};

#endif  // _ASP_BASIC_IO_HPP_