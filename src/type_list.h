#pragma once

#include <type_traits>
#include <variant>

namespace meta {

template<class... Ts>
struct type_list {
    using variant = std::variant<Ts...>;
};

template<class TypeList, class T>
struct type_list_contains_t : std::false_type {};

template<class T, class... Ts>
struct type_list_contains_t<type_list<T, Ts...>, T> : std::true_type {};

template<class T, class T1, class... Ts>
struct type_list_contains_t<type_list<T1, Ts...>, T> : type_list_contains_t<type_list<Ts...>, T> {};

template<class TypeList, class T>
inline constexpr bool type_list_contains = type_list_contains_t<TypeList, T>::value;

}// namespace meta
