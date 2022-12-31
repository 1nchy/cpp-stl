#ifndef _LOG_UTILS_HPP_
#define _LOG_UTILS_HPP_

#include <cstdio>
#include <cstring>

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

#endif // _LOG_UTILS_HPP_