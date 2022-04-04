#pragma once

namespace util {

template<class... L>
struct overload : L... {
    using L::operator()...;
};
template<class... L>
overload(L...) -> overload<L...>;

}// namespace util
