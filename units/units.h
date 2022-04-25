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

// Unit multiplication
template <typename TFirstUnit, typename ... TOtherUnits>
struct _multiplied_unit
{};

template <typename TFirstUnit>
struct _multiplied_unit<TFirstUnit>
{
    using type = TFirstUnit;
};

template <typename TUnitEnum, typename TFirstPowers, typename TSecondPowers, typename ... TOtherUnits>
struct _multiplied_unit<unit<TUnitEnum, TFirstPowers>, unit<TUnitEnum, TSecondPowers>, TOtherUnits ...>
{
    using type = typename _multiplied_unit<unit<TUnitEnum, add_t<TFirstPowers, TSecondPowers>>, TOtherUnits ...>::type;
};

template <typename TFirstUnit, typename ... TOtherUnits>
using multiplied_unit = typename _multiplied_unit<TFirstUnit, TOtherUnits ...>::type;

// Unit division
template <typename TDividendUnit, typename TDivisorUnit>
struct _divided_unit
{};

template <typename TUnitEnum, typename TDividendPowers, typename TDivisorPowers>
struct _divided_unit<unit<TUnitEnum, TDividendPowers>, unit<TUnitEnum, TDivisorPowers>>
{
    using type = unit<TUnitEnum, subtract_t<TDividendPowers, TDivisorPowers>>;
};

template <typename TDividendUnit, typename TDivisorUnit>
using divided_unit = typename _divided_unit<TDividendUnit, TDivisorUnit>::type;

// Operator + 
template <typename TUnit, typename TValue>
quantity<TUnit, TValue> operator+(quantity<TUnit, TValue> lhs, quantity<TUnit, TValue> rhs)
{
    return quantity<TUnit, TValue>(lhs.value() + rhs.value());
}

// Operator -
template <typename TUnit, typename TValue>
quantity<TUnit, TValue> operator-(quantity<TUnit, TValue> lhs, quantity<TUnit, TValue> rhs)
{
    return quantity<TUnit, TValue>(lhs.value() - rhs.value());
}

// Operator *
template <typename TFirstUnit, typename TSecondUnit, typename TValue = double>
quantity<multiplied_unit<TFirstUnit, TSecondUnit>, TValue> operator*(quantity<TFirstUnit, TValue> lhs, quantity<TSecondUnit, TValue> rhs)
{
    return quantity<multiplied_unit<TFirstUnit, TSecondUnit>, TValue>(lhs.value() * rhs.value());
}

// Operator /
template <typename TFirstUnit, typename TSecondUnit, typename TValue> 
quantity<divided_unit<TFirstUnit, TSecondUnit>, TValue> operator/(quantity<TFirstUnit, TValue> lhs, quantity<TSecondUnit, TValue> rhs)
{
    return quantity<divided_unit<TFirstUnit, TSecondUnit>, TValue>(lhs.value() / rhs.value());
}

#endif