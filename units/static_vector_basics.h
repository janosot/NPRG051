#include <iostream>
#include <limits>

#ifndef VECTOR_H
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
template <std::size_t Size, int Value, int ... Values>
struct fill_impl
{
    using type = push_back_t<typename fill_impl<Size - 1, Value, Values ...>::type, Value>;
};

template <int Value, int ... Values>
struct fill_impl<0, Value, Values ...>
{
    using type = static_vector<Value, Values ...>;
};

template <std::size_t Size, int Value>
struct fill
{
    using type = typename fill_impl<Size, Value>::type;
};

template <std::size_t Size>
struct fill<Size, 0>
{
    using type = typename fill_impl<Size - 1, 0>::type;
};

template <std::size_t Size, int Value>
using fill_t = typename fill<Size, Value>::type;

// Set a value at a specific index of the static_vector
template <typename OldVector, typename NewVector, std::size_t Index, int Value>
struct set_impl
{};

template <typename NewVector, std::size_t Index, int Value, int OldHead, int ... OldTail>
struct set_impl<static_vector<OldHead, OldTail ...>, NewVector, Index, Value>
{
    using type = typename set_impl<static_vector<OldTail ...>, push_back_t<NewVector, OldHead>, Index - 1, Value>::type;
};

template <typename NewVector, int Value, int OldHead, int ... OldTail>
struct set_impl<static_vector<OldHead, OldTail ...>, NewVector, 0, Value>
{
    using type = typename set_impl<static_vector<OldTail ...>, push_back_t<NewVector, Value>, std::numeric_limits<std::size_t>::max(), Value>::type;
};

template <typename NewVector, int Value, int OldHead, int ... OldTail>
struct set_impl<static_vector<OldHead, OldTail ...>, NewVector, std::numeric_limits<std::size_t>::max(), Value>
{
    using type = typename set_impl<static_vector<OldTail ...>, push_back_t<NewVector, OldHead>, std::numeric_limits<std::size_t>::max(), Value>::type;
};

template <typename NewVector, int Value>
struct set_impl<static_vector<>, NewVector, std::numeric_limits<std::size_t>::max(), Value>
{
    using type = NewVector;
};

template <typename Vector, std::size_t Index, int Value>
struct set
{
    using type = typename set_impl<Vector, static_vector<>, Index, Value>::type;
};

template <typename Vector, std::size_t Index, int Value>
using set_t = typename set<Vector, Index, Value>::type;

// Add two vectors
template <typename FirstVector, typename SecondVector, typename AddedVector>
struct add_impl
{};

template <int FirstHead, int SecondHead, typename AddedVector>
struct add_impl<static_vector<FirstHead>, static_vector<SecondHead>, AddedVector>
{
    using type = push_back_t<AddedVector, (FirstHead + SecondHead)>;
};

template <int FirstHead, int ... FirstTail, int SecondHead, int ... SecondTail, typename AddedVector>
struct add_impl<static_vector<FirstHead, FirstTail ...>, static_vector<SecondHead, SecondTail ...>, AddedVector>
{
    using type = typename add_impl<static_vector<FirstTail ...>, static_vector<SecondTail ...>, push_back_t<AddedVector, (FirstHead + SecondHead)>>::type;
};

template <typename FirstVector, typename SecondVector>
struct add
{
    using type = typename add_impl<FirstVector, SecondVector, static_vector<>>::type;
};

template <typename FirstVector, typename SecondVector>
using add_t = typename add<FirstVector, SecondVector>::type;

// Subtract two vectors
template <typename FirstVector, typename SecondVector, typename SubtractedVector>
struct subtract_impl
{};

template <int FirstHead, int SecondHead, typename AddedVector>
struct subtract_impl<static_vector<FirstHead>, static_vector<SecondHead>, AddedVector>
{
    using type = push_back_t<AddedVector, (FirstHead - SecondHead)>;
};

template <int FirstHead, int ... FirstTail, int SecondHead, int ... SecondTail, typename SubtractedVector>
struct subtract_impl<static_vector<FirstHead, FirstTail ...>, static_vector<SecondHead, SecondTail ...>, SubtractedVector>
{
    using type = typename subtract_impl<static_vector<FirstTail ...>, static_vector<SecondTail ...>, push_back_t<SubtractedVector, (FirstHead - SecondHead)>>::type;
};

template <typename FirstVector, typename SecondVector>
struct subtract
{
    using type = typename subtract_impl<FirstVector, SecondVector, static_vector<>>::type;
};

template <typename FirstVector, typename SecondVector>
using subtract_t = typename subtract<FirstVector, SecondVector>::type;

#endif