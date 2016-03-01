/* Copyright (C) 2016 Murray Cumming
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef MURRAYC_TUPLE_UTILS_TUPLE_FOR_EACH_H
#define MURRAYC_TUPLE_UTILS_TUPLE_FOR_EACH_H

#include <tuple>

namespace sigc {

namespace detail {

template <template <typename> class T_visitor, std::size_t index,
  typename... T_extras>
struct tuple_for_each_impl {
  template <typename T>
  static void
  tuple_for_each(T&& t, T_extras&&... extras) {
    using element_type = typename std::tuple_element<index, std::decay_t<T>>::type;
    T_visitor<element_type>::visit(std::get<index>(t), std::forward<T_extras>(extras)...);

    tuple_for_each_impl<T_visitor, index - 1, T_extras...>::tuple_for_each(
      std::forward<T>(t), std::forward<T_extras>(extras)...);
  }
};

template <template <typename> class T_visitor, typename... T_extras>
struct tuple_for_each_impl<T_visitor, 0, T_extras...> {
  template <typename T>
  static void
  tuple_for_each(T&& t, T_extras&&... extras) {
    constexpr std::size_t index = 0;

    using element_type = typename std::tuple_element<index, std::decay_t<T>>::type;
    T_visitor<element_type>::visit(std::get<index>(std::forward<T>(t)), std::forward<T_extras>(extras)...);
  }
};

} // detail namespace


/**
 * Get a tuple with each element having the transformed value of the element
 * in the original tuple.
 *
 * @tparam T_visitor should be a template that has a static visit() method.
 * @tparam T the tuple type.
 * @tparam T_extras the types of any extra arguments to pass to @e T_Visitor's
 * visit() method.
 * @param t The tuple whose elements should be visited.
 * @param extras Any extra arguments to pass to @e T_Visitor's visit() method.
 */
template <template <typename> class T_visitor, typename T, typename... T_extras>
void
tuple_for_each(T&& t, T_extras&&... extras) {
  //We use std::decay_t<> because tuple_size is not defined for references.
  constexpr auto size = std::tuple_size<std::decay_t<T>>::value;

  if(size == 0) {
    return;
  }

  detail::tuple_for_each_impl<T_visitor, size - 1, T_extras...>::tuple_for_each(
    std::forward<T>(t), std::forward<T_extras>(extras)...);
}

} // namespace sigc

#endif //MURRAYC_TUPLE_UTILS_TUPLE_FOR_EACH_H
