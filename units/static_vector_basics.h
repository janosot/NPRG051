#include <iostream>

#ifndef VECTOR__H
#define VECTOR_H 

template <int ... Values>
struct static_vector
{};

// operation push_back - adds an element to the end of the vector
template <typename Vector, int Added>
struct push_back
{};

template <int Added, int ... Values>
struct push_back<static_vector<Values ...>, Added>
{
    using type = static_vector<Values ..., Added>;
};

template <typename Vector, int Added>
using push_back_t = typename push_back<Vector, Added>::type;

// operation at:: Get value at an index
template <typename Vector, std::size_t Index>
struct at
{};

template <int Head, int ... Tail>
struct at<static_vector<Head, Tail ...>, 0>
{
    static constexpr auto value = Head;
};

template <std::size_t Index, int Head, int ... Tail>
struct at<static_vector<Head, Tail ...>, Index>
{
    static constexpr auto value = at<static_vector<Tail ...>, Index - 1>::value;
};

// Create a vector with specific size and initial value
template <std::size_t Size, int Value>
struct fill
{
    using type = push_back_t<typename fill<Size - 1, Value>::type, Value>;
};

template <std::size_t Size>
struct fill<Size, 0>
{
    using type = static_vector<>;
};

template <std::size_t Size, int Value>
using fill_t = typename fill<Size, Value>::type;

// Join two vectors together
template <typename FirstVector, typename SecondVector>
struct join
{};

template <int ... FirstValues, int ... SecondValues>
struct join<static_vector<FirstValues ...>, static_vector<SecondValues ...>>
{
    using type = static_vector<FirstValues ..., SecondValues ...>;
};

template <typename FirstVector, typename SecondVector>
using join_t = typename join<FirstVector, SecondVector>::type;

// Set a value at a specific index of the static_vector
template <typename Vector, std::size_t Index, int Value>
struct set
{};

template <int Value, int ... Values, std::size_t Index>
struct set<static_vector<Values ...>, Index, Value>
{
    using type = static_vector<Values ..., Value>;
};

template <typename Vector, std::size_t Index, int Value>
using set_t = typename set<Vector, Index, Value>::type;

// Add two vectors
template <typename Vector1, typename Vector2>
struct add
{};

template <int ... Values1, int ... Values2>
struct add<static_vector<Values1 ...>, static_vector<Values2 ...>>
{
    using type = static_vector<Values1 ..., Values2 ...>;
};

template <typename Vector1, typename Vector2>
using add_t = typename add<Vector1, Vector2>::type;

// Subtract two vectors
template <typename Vector1, typename Vector2>
struct subtract
{};

template <int ... Values1, int ... Values2>
struct subtract<static_vector<Values1 ...>, static_vector<Values2 ...>>
{
    using type = static_vector<Values1 ..., Values2 ...>;
};

template <typename Vector1, typename Vector2>
using subtract_t = typename subtract<Vector1, Vector2>::type;

#endif