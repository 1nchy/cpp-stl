#ifndef _ASP_DEBUG_HPP_
#define _ASP_DEBUG_HPP_

#include <ostream>

template <typename _Tp> std::ostream& operator<<(std::ostream& os, const _Tp& rhs);
template <typename _Tp, typename... Args> std::ostream& operator<<(std::ostream& os, Args... args);

#endif