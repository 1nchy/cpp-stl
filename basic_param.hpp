#ifndef _ASP_BASIC_PARAM_HPP_
#define _ASP_BASIC_PARAM_HPP_

namespace asp {
    
typedef unsigned int size_type;
typedef signed int difference_type;

typedef enum {
    __FAILED__,
    __WRONG__,
    __PAUSED__,
    __NORMAL__,
    __COMPLETED__,
} task_status;

#define _CONTAINER_CHECK_ 1

};

#endif