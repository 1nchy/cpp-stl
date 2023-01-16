#ifndef _ASP_RANDOM_HPP_
#define _ASP_RANDOM_HPP_

#include <ctime>
#include <cstdlib>
#include <cstdarg>
#include <initializer_list>

#include "basic_param.hpp"
#include "log_utils.hpp"

namespace asp {
    static void _S_init_random_seed();
    constexpr static const size_type _s_percentage_base = 100;
    // end with 0
    double rand_float() { _S_init_random_seed(); return (double)rand() / RAND_MAX; }

    size_type _S_random_unsigned(double _n1, ...);
    size_type _S_random_unsigned(std::initializer_list<double> _il);
    template <size_type _N> size_type _S_random_unsigned(const std::array<double, _N>& _a);



/// implement
    size_type _S_random_unsigned(double _n1, ...) {
        _S_init_random_seed();
        va_list _l;
        va_start(_l, _n1);
        size_type _r = rand() % _s_percentage_base;
        size_type _sum = 0;
        size_type _ret = 0;
        for (size_type _i = _n1 * _s_percentage_base; _i != 0; _i = size_type(va_arg(_l, double) * _s_percentage_base), ++_ret) {
            _sum += _i;
            if (_sum > _r) {
                return _ret;
            }
        }
        va_end(_l);
        return _ret;
    }
    size_type _S_random_unsigned(std::initializer_list<double> _il) {
        _S_init_random_seed();
        size_type _r = rand() % _s_percentage_base;
        size_type _sum = 0;
        size_type _ret = 0;
        for (auto _it = _il.begin(); _it != _il.end(); ++_it, ++_ret) {
            _sum += (*_it * _s_percentage_base);
            if (_sum > _r) {
                return _ret;
            }
        }
        return _ret;
    }
    template <size_type _N> size_type _S_random_unsigned(const std::array<double, _N>& _a) {
        _S_init_random_seed();
        size_type _r = rand() % _s_percentage_base;
        size_type _sum = 0;
        for (size_type _i = 0; _i < _N; ++_i) {
            _sum += (_a[_i] * _s_percentage_base);
            if (_sum > _r) {
                return _i;
            }
        }
        return _N;
    }


/// random seed initialization
    static void _S_init_random_seed() {
        static bool _seed_inited = false;
        if (!_seed_inited) {
            srand((uint)time(nullptr));
            _seed_inited = true;
            // ASP_LOG("random seed initialized.\n");
        }
    }
};

#endif // _ASP_RANDOM_HPP_