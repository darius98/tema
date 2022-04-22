#pragma once

#include <type_traits>
#include <variant>
#include <tuple>

namespace util {

template<class...>
struct tpack {
};

template<class TypeList, class T>
struct tpack_contains_t : std::false_type {};

template<class T, class... Ts>
struct tpack_contains_t<tpack<T, Ts...>, T> : std::true_type {};

template<class T, class T1, class... Ts>
struct tpack_contains_t<tpack<T1, Ts...>, T> : tpack_contains_t<tpack<Ts...>, T> {};

template<class TypeList, class T>
inline constexpr bool tpack_contains = tpack_contains_t<TypeList, T>::value;

template<class T, class TypeList>
concept one_of_pack = tpack_contains<TypeList, T>;

template<class T, class... Ts>
concept one_of = one_of_pack<T, tpack<Ts...>>;

template<class T>
struct for_tpack;

template<class... Ts>
struct for_tpack<tpack<Ts...>> {
    using variant_type = std::variant<Ts...>;
    using tuple_type = std::tuple<Ts...>;
};

template<class T>
using variant_for = typename for_tpack<T>::variant_type;

template<class T>
using tuple_for = typename for_tpack<T>::tuple_type;

}// namespace util
