#pragma once

#include <type_traits>
#include <variant>

namespace util {

template<class...>
struct pack {
};

template<class TypeList, class T>
struct pack_contains_t : std::false_type {};

template<class T, class... Ts>
struct pack_contains_t<pack<T, Ts...>, T> : std::true_type {};

template<class T, class T1, class... Ts>
struct pack_contains_t<pack<T1, Ts...>, T> : pack_contains_t<pack<Ts...>, T> {};

template<class TypeList, class T>
inline constexpr bool pack_contains = pack_contains_t<TypeList, T>::value;

template<class T>
struct variant_for_pack_impl;

template<class... Ts>
struct variant_for_pack_impl<pack<Ts...>> {
    using type = std::variant<Ts...>;
};

template<class T>
using variant_for_pack = typename variant_for_pack_impl<T>::type;

}// namespace util
