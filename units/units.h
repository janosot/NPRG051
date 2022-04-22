#include <cstdint>
#include <type_traits>
#include <cstdio>

#ifndef UNITS_H
#define UNITS_H 

#include "static_vector_basics.h"

template <typename TEnum, typename TPowers>
struct unit;

// Template containing an enum specifying the set of base units and a static vector of exponents of these units.
template <typename TEnum, TEnum index>
using basic_unit = unit<TEnum, set_t<fill_t<(std::size_t)TEnum::_count, 0>, (std::size_t)index, 1>>;

// Unit multiplication
template <typename TFirstUnit, typename ... TOtherUnits>
struct multiplied_unit
{};

template <typename TFirstUnit>
struct multiplied_unit<TFirstUnit>
{
    using type = TFirstUnit;
};

template <typename TUnitEnum, typename TFirstPowers, typename TSecondPowers>
struct multiplied_unit<unit<TUnitEnum, TFirstPowers>, unit<TUnitEnum, TSecondPowers> >
{
    using type = unit<TUnitEnum, add_t<TFirstPowers, TSecondPowers>>;
};

// Unit division 
// A type definition that creates a type instantiation of unit over a given system of units, the resulting vector will contain the sum of the vectors of all specified units.
template <typename TDividendUnit, typename TDivisorUnit>
struct divided_unit
{};

template <typename TUnitEnum, typename TDividendPowers, typename TDivisorPowers>
struct divided_unit<unit<TUnitEnum, TDividendPowers>, unit<TUnitEnum, TDivisorPowers> >
{
    using type = unit<TUnitEnum, subtract_t<TDividendPowers, TDivisorPowers>>;
};

// A type definition that creates a type instantiation of unit over a given system of units,
// the resulting vector will contain the difference of the vectors of the specified units.
template <typename TUnit, typename TValue = double>
class quantity
{
private:
    TValue _value;

public:
    using value_type = TValue;
    explicit quantity(value_type value) : _value(value) {}
    value_type value() const { return _value; }
};

//operator +
template <typename TUnit1, typename TUnit2, typename TValue>
inline quantity<multiplied_unit<TUnit1, TUnit2>, TValue> operator+(quantity<TUnit1, TValue> lhs, quantity<TUnit2, TValue> rhs)
{
    return quantity<multiplied_unit<TUnit1, TUnit2>, TValue>(lhs.value() + rhs.value());
}

// operator -
template <typename TUnit1, typename TUnit2, typename TValue>
inline quantity<multiplied_unit<TUnit1, TUnit2>, TValue> operator-(quantity<TUnit1, TValue> lhs, quantity<TUnit2, TValue> rhs)
{
    return quantity<multiplied_unit<TUnit1, TUnit2>, TValue>(lhs.value() - rhs.value());
}

// operator /
template <typename TUnit1, typename TUnit2, typename TValue>
inline quantity<divided_unit<TUnit1, TUnit2>, TValue> operator/(const quantity<TUnit1, TValue>& q1, const quantity<TUnit2, TValue>& q2)
{
    return quantity<divided_unit<TUnit1, TUnit2>, TValue> (q1.value() / q2.value());
}

// operator *
template <typename TUnit1, typename TUnit2, typename TValue>
inline quantity<multiplied_unit<TUnit1, TUnit2>, TValue> operator*(const quantity<TUnit1, TValue>& q1, const quantity<TUnit2, TValue>& q2)
{
    return quantity<multiplied_unit<TUnit1, TUnit2>, TValue> (q1.value() * q2.value());
}

#endif
