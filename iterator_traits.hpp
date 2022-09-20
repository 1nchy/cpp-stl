#ifndef _ASP_ITERATOR_TRAITS_HPP_
#define _ASP_ITERATOR_TRAITS_HPP_

#include "basic_param.hpp"

namespace asp {

struct fixed_iterator {}; // support * ->
struct forward_iterator_tag : public fixed_iterator {}; // support +
struct backward_iterator_tag : public fixed_iterator {}; // support -
struct bidirectional_iterator_tag : public forward_iterator_tag, public backward_iterator_tag {}; // support + and -
struct random_access_iterator_tag : public bidirectional_iterator_tag {}; // support self- and +n

template <typename _Iter> class iterator_traits;

template <typename _Iter> class iterator_traits {
public:
    typedef typename _Iter::iterator_category iterator_category;
    typedef typename _Iter::value_type value_type;
    typedef typename _Iter::pointer pointer;
    typedef typename _Iter::reference reference;
    typedef typename _Iter::difference_type difference_type;
};

template <typename _Tp> class iterator_traits <_Tp*> {
public:
    typedef random_access_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef _Tp* pointer;
    typedef _Tp& reference;
    typedef asp::difference_type difference_type;
};

template <typename _Tp> class iterator_traits <const _Tp*> {
public:
    typedef random_access_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef const _Tp* pointer;
    typedef const _Tp& reference;
    typedef asp::difference_type difference_type;
};

};

#endif