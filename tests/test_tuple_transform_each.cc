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

#include <sigc++/tuple_transform_each.h>
#include <utility>
#include <cstdlib>
#include <cassert>


template <class T_element_from>
class transform_to_string
{
public:
  static
  decltype(auto)
  transform(T_element_from& from) {
    return std::to_string(from);
  } 
};

/*
void test_tuple_type_transform_each_same_types()
{
  using type_tuple_original = std::tuple<int, int>;
  using type_tuple_transformed = sigc::tuple_type_transform_each<
    type_tuple_original, transform_to_string>::type;
  using type_tuple_expected = std::tuple<std::string, std::string>;

  static_assert(std::is_same<type_tuple_transformed, type_tuple_expected>::value,
    "unexpected tuple_transform_each()ed tuple type");
}
*/

//In these tests, t_expected has elements all of the same type.
void test_tuple_transform_each_same_types()
{
  {
    auto t_original = std::make_tuple(1, 2, 3);
    auto t_transformed = sigc::tuple_transform_each<transform_to_string>(t_original);
    auto t_expected = std::make_tuple(std::string("1"), std::string("2"), std::string("3"));

    static_assert(std::tuple_size<decltype(t_transformed)>::value == 3,
      "unexpected tuple_transform_each()ed tuple size.");

    assert(std::get<0>(t_transformed) == "1");
    assert(std::get<1>(t_transformed) == "2");
    assert(std::get<2>(t_transformed) == "3");

    static_assert(std::is_same<decltype(t_transformed), decltype(t_expected)>::value,
      "unexpected transform_each()ed tuple type");
  }

  {
    auto t_original = std::make_tuple(1, (double)2.1f, 3);
    auto t_transformed = sigc::tuple_transform_each<transform_to_string>(t_original);
    auto t_expected = std::make_tuple(std::string("1"), std::string("2"), std::string("3"));

    static_assert(std::tuple_size<decltype(t_transformed)>::value == 3,
      "unexpected tuple_transform_each()ed tuple size.");

    assert(std::get<0>(t_transformed) == "1");
    assert(std::get<1>(t_transformed).substr(0, 3) == "2.1");
    assert(std::get<2>(t_transformed) == "3");

    static_assert(std::is_same<decltype(t_transformed), decltype(t_expected)>::value,
      "unexpected transform_each()ed tuple type");
  }
}

//The general template declaration.
//We then provide specializations for each type,
//so we can test having a different return value for each T_element_from type.
template <class T_element_from>
class transform_to_something;

//An int will be converted to a std::string:
template <>
class transform_to_something<int>
{
public:
  static
  std::string
  transform(int& from) {
    return std::to_string(from);
  } 
};

//A double will be converted to a char:
template <>
class transform_to_something<double>
{
public:
  static
  char
  transform(double& from) {
    return std::to_string(from)[0];
  } 
};

//A std::string will be converted to an int:
template <>
class transform_to_something<std::string>
{
public:
  static
  int
  transform(std::string& from) {
    return std::stoi(from);
  } 
};

/*
void test_tuple_type_transform_each_multiple_types()
{
  using type_tuple_original = std::tuple<int, double, std::string>;
  using type_tuple_transformed = sigc::tuple_type_transform_each<
    type_tuple_original, transform_to_something>::type;
  using type_tuple_expected = std::tuple<std::string, char, int>;

  static_assert(std::is_same<type_tuple_transformed, type_tuple_expected>::value,
    "unexpected tuple_transform_each()ed tuple type");
}
*/

//In these tests, t_expected has elements of different types.
void test_tuple_transform_each_multiple_types()
{
  auto t_original = std::make_tuple(1, (double)2.1f, std::string("3"));
  auto t_transformed = sigc::tuple_transform_each<transform_to_something>(t_original);
  auto t_expected = std::make_tuple(std::string("1"), '2', 3);

  static_assert(std::tuple_size<decltype(t_transformed)>::value == 3,
    "unexpected tuple_transform_each()ed tuple size.");

  assert(std::get<0>(t_transformed) == "1");
  assert(std::get<1>(t_transformed) == '2');
  assert(std::get<2>(t_transformed) == 3);

  static_assert(std::is_same<decltype(t_transformed), decltype(t_expected)>::value,
    "unexpected transform_each()ed tuple type");
}


template <class T_element_from>
class transform_each_nonconst
{
public:
  static
  int
  transform(T_element_from& from) {
    from *= 2;
    //Or, for instance, call a non-const method on from.

    return from * 10;
  } 
};

void test_tuple_transform_each_nonconst()
{
  auto t = std::make_tuple(1, 2, 3);
  auto t_transformed = sigc::tuple_transform_each<transform_each_nonconst>(t);

  //Check that t was changed (from * 2):
  assert(std::get<0>(t) == 2);
  assert(std::get<1>(t) == 4);
  assert(std::get<2>(t) == 6);

  //Check that t_transformed has the expected values ( from * 2 * 10):
  assert(std::get<0>(t_transformed) == 20);
  assert(std::get<1>(t_transformed) == 40);
  assert(std::get<2>(t_transformed) == 60);
}


int main()
{
  //test_tuple_type_transform_each_same_types();

  //test_tuple_type_transform_each_multiple_types();

  test_tuple_transform_each_same_types();
  test_tuple_transform_each_multiple_types();

  test_tuple_transform_each_nonconst();
      
  return EXIT_SUCCESS;
}
