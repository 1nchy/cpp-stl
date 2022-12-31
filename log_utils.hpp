#ifndef _LOG_UTILS_HPP_
#define _LOG_UTILS_HPP_

#include <cstdio>
#include <cstring>

#include <ostream>
#include <vector>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
// #define __FILENAME__ __FILE__

#define __STD_OUTPUT__ stdout
#define __STD_ERROR__ stderr

#define ASP_LOG(format, ...)\
 fprintf(__STD_OUTPUT__, "[%s][%s][%d]msg: " format "\n",\
 __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ASP_ERR(format, ...)\
 fprintf(__STD_ERROR__, "[%s][%s][%d]err: " format "\n",\
 __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)


template <typename _Tp, typename _Alloc> std::ostream& operator<<(std::ostream& _os, const std::vector<_Tp, _Alloc> _v) {
    _os << '[';
    for (auto _p = _v.cbegin(); _p != _v.cend(); ++_p) {
        _os << *_p;
        if (_p + 1 != _v.cend()) {
            _os << ", ";
        }
    }
    _os << ']';
    return _os;
}

#endif // _LOG_UTILS_HPP_