#include "static_ranges.hpp"
#include <functional>
#include <iostream>
#include <iomanip>
#include <variant>

int main(int argc, char** argv)
{
	   auto increment_ftor = std::bind(std::plus(), std::placeholders::_1, 1);
       auto onetwo_lvalue_ftor = static_views::iota<int, 2>() | static_views::transform(increment_ftor);
       //print("onetwo_lvalue_ftor", onetwo_lvalue_ftor);

       std::variant<decltype(increment_ftor), std::array<char, sizeof(decltype(increment_ftor))>> spoiled_increment_ftor(increment_ftor);
       auto onetwo_spoiled_lvalue_ftor = static_views::iota<int, 2>() | static_views::transform(std::get<0>(spoiled_increment_ftor));
       //print("onetwo_spoiled_lvalue_ftor before", onetwo_spoiled_lvalue_ftor);
       spoiled_increment_ftor.emplace<1>();
       //print("onetwo_spoiled_lvalue_ftor after", onetwo_spoiled_lvalue_ftor);

       auto onetwo_nonempty_view_lvalue_ftor = static_views::all(onetwo_lvalue_ftor) | static_views::transform(increment_ftor);
       //print("onetwo_nonempty_view_lvalue_ftor", onetwo_nonempty_view_lvalue_ftor);
}

/**/