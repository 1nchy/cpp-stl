#ifndef _ASP_FUNCTIONAL_HPP_
#define _ASP_FUNCTIONAL_HPP_

#include <memory>
#include <utility>

namespace asp {

template<typename _T> class function;
template<typename _R, typename... _Args> class function<_R(_Args...)> {
public:
    template<typename _T> function(_T&& t):
        _call(std::make_unique<_Callable<_T>>(t)) {}
    _R operator()(_Args&&... _args) {
        return _call->invoke(std::forward<_Args>(_args)...);
    }
private:
    class _Base {
    public:
        virtual ~_Base() = default;
        virtual _R invoke(_Args&&... _args) = 0;
    };
    template<typename _T> class _Callable : public _Base {
    public:
        _Callable(const _T& t): _t(t) {}
        ~_Callable() override = default;
        _R invoke(_Args&&... _args) override {
            return _t(std::forward<_Args>(_args)...);
        }
    private:
        _T _t;
    };
    std::unique_ptr<_Base> _call;
};

};

#endif