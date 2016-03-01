#ifndef _SIGC_TUPLE_CDR_H_
#define _SIGC_TUPLE_CDR_H_

#include <tuple>
#include <utility>
#include <type_traits>

namespace sigc
{

namespace {

template<typename T, typename Seq>
struct tuple_type_cdr_impl;

template<typename T, std::size_t I0, std::size_t... I>
struct tuple_type_cdr_impl<T, std::index_sequence<I0, I...>>
{
  using type = std::tuple<typename std::tuple_element<I, T>::type...>;
};

} //anonymous namespace

/**
 * Get the type of a tuple without the first item.
 */
template<typename T>
struct tuple_type_cdr
: tuple_type_cdr_impl<T, std::make_index_sequence<std::tuple_size<T>::value>>
{};


namespace {

template<typename T, std::size_t I0, std::size_t... I>
decltype(auto) tuple_cdr_impl(const T& t, std::index_sequence<I0, I...>) {
  return std::make_tuple(std::get<I>(t)...);
}

} //anonymous namespace

/**
 * Get the a tuple without the first item.
 * This is analogous to std::tuple_cat().
 */
template<typename T>
decltype(auto) tuple_cdr(const T& t) {
  constexpr auto size = std::tuple_size<T>::value;
  const auto seq = std::make_index_sequence<size>{};
  return tuple_cdr_impl(t, seq);
}


} //namespace sigc

#endif //_SIGC_TUPLE_CDR_H_
