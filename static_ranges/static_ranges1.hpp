#ifndef _STATIC_RANGES_H
#define _STATIC_RANGES_H

#include <utility>
#include <tuple>
#include <array>
#include <cstdint>
#include <type_traits>

namespace static_ranges
{
    template <typename T> struct range_traits;
    struct view_base {};

    // concepts
    namespace detail
    {
        template <typename, typename = void>
        static constexpr bool is_complete_v = false;

        template <typename T>
        static constexpr bool is_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;
    }

    template <typename T>
    concept range = requires(T r)
    {
        detail::is_complete_v<range_traits<std::remove_cvref_t<T>>>;
        std::is_same_v<decltype(static_ranges::range_traits<std::remove_cvref_t<T>>::value), std::size_t>;
    };

    template <typename T>
    concept view = range<T> && std::is_base_of_v<view_base, T>;

    // convenience
    template <typename T>
    static constexpr std::size_t size_v = static_cast<std::size_t>(range_traits<std::remove_cvref_t<T>>::value);

    template <typename T, typename U>
    static constexpr std::size_t size_v<std::pair<T, U>> = 2;

    template <typename ... Pack>
    static constexpr std::size_t size_v<std::tuple<Pack ...>> = sizeof...(Pack);

    template <std::size_t N, typename T>
    static constexpr std::size_t size_v<std::array<T, N>> = N;

    template <std::size_t I>
    constexpr auto&& element(range auto&& r)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(r)>)
        {
            return range_traits<std::remove_cvref_t<decltype(r)>>::template get<I>(std::move(r));
        }
        else
        {
            return range_traits<std::remove_cvref_t<decltype(r)>>::template get<I>(r);
        }
    }

    template <std::size_t I, typename U, typename V>
    constexpr auto&& element(std::pair<U, V>&& r)
    {
        static_assert(I >= 0 && I <= 1);
        if constexpr (I == 0)
        {
            return std::forward<U>(r.first);
        }
        else
        {
            return std::forward<V>(r.second);
        }
    }

    template <std::size_t I, typename ... Pack>
    constexpr auto&& element(std::tuple<Pack ...> r)
    {
        static_assert(I >= 0 && I <= sizeof...(Pack));
        return std::get<I>(r);
    }

    // static_ranges::copy
    // copies the static range r1 into the static range r2
    template<std::size_t I, std::size_t Size>
    constexpr void copy_impl(range auto&& r1, range auto&& r2)
    {
        if constexpr (I < Size)
        {
            element<I>(r2) = element<I>(r1);
            copy_impl<I + 1, Size>(r1, r2);
        }
    }

    constexpr void copy(range auto&& r1, range auto&& r2)
    {
        copy_impl<0, size_v<std::remove_cvref_t<decltype(r1)>>>(std::forward<decltype(r1)>(r1), std::forward<decltype(r2)>(r2));
    }

    // for_each
    // applies f(e) to every element e of the static range r
    template<std::size_t I, std::size_t Size>
    constexpr void for_each_impl(range auto&& r, auto f)
    {
        if constexpr (I < Size)
        {
            f(element<I>(r));
            for_each_impl<I + 1, Size>(std::forward<decltype(r)>(r), f);
        }
    }

    constexpr void for_each(range auto&& r, auto f)
    {
        for_each_impl<0, size_v<std::remove_cvref_t<decltype(r)>>>(std::forward<decltype(r)>(r), f);
    }


    namespace views
    {
        // all_view
        template<range R>
        struct all_view : view_base
        {
            constexpr all_view(R& r)
                : ref(r)
            {}

            operator R& ()
            {
                return ref;
            }
            R& ref;
        };

        template<typename RV>
        auto all(RV& rv)
        {
            if constexpr (view<RV>)
            {
                return rv;
            }
            else
            {
                return all_view<RV>(rv);
            }
        };

        // iota_view
        template<typename T, std::size_t N>
        struct iota_view : view_base
        {};

        template<typename T, std::size_t N>
        auto iota()
        {
            static_assert(std::is_integral_v<T>);
            return iota_view<T, N>();
        }

        // static_iota_view
        template<typename T, std::size_t N>
        struct static_iota_view : view_base
        {};

        template<typename T, std::size_t N>
        auto static_iota()
        {
            static_assert(std::is_integral_v<T>);
            return static_iota_view<T, N>();
        }
    }

    // view traits
    template<range R>
    struct range_traits<views::all_view<R>> : std::integral_constant<std::size_t, size_v<R>>
    {
        template<std::size_t I>
        static auto&& get(views::all_view<R>& v)
        {
            if constexpr (I < size_v<R>) return range_traits<std::remove_cvref_t<R>>::template get<I>(v.ref);
        }
    };

    template<typename T, std::size_t N>
    struct range_traits<views::iota_view<T, N>> : std::integral_constant<std::size_t, N>
    {
        template<std::size_t I>
        static auto get(views::iota_view<T, N>)
        {
            if constexpr (I < N) return T(I);
        }
    };

    template<typename T, std::size_t N>
    struct range_traits<views::static_iota_view<T, N>> : std::integral_constant<std::size_t, N>
    {
        template<std::size_t I>
        static auto get(views::static_iota_view<T, N>)
        {
            if constexpr (I < N) return std::integral_constant<T, I>();
        }
    };

    // to_tuple
    template<std::size_t I, std::size_t Size>
    constexpr auto to_tuple_impl(range auto&& r)
    {
        if constexpr (I < Size)
        {
            return std::tuple_cat(std::make_tuple(element<I>(r)), to_tuple_impl<I + 1, Size>(std::forward<decltype(r)>(r)));
        }
        else
        {
            return std::tuple<>{};
        }
    }

    constexpr auto to_tuple(range auto&& r)
    {
        return to_tuple_impl<0, size_v<std::remove_cvref_t<decltype(r)>>>(std::forward<decltype(r)>(r));
    }

    // to_pair
    template <typename T>
    constexpr auto to_pair(T&& r)
    {
        assert (size_v<T> == 2);
        return std::pair<decltype(element<0>(r)), decltype(element<1>(r))>(element<0>(r), element<1>(r));
    }

    // to_array
    template<typename T, range R>
    inline constexpr auto to_array(R&& r)
    {
        std::array<T, size_v<std::remove_cvref_t<R>>> arr;
        copy(r, arr);
        return arr;
    }

    // std
    template<typename T, typename U>
    struct range_traits<std::pair<T, U>> : std::integral_constant<std::size_t, 2>
    {
        template<std::size_t I>
        static auto&& get(std::pair<T, U>& r)
        {
            if constexpr (I == 0)
            {
                return r.first;
            }
            else
            {
                return r.second;
            }
        }

        template<std::size_t I>
        static auto&& get(std::pair<T, U>&& r)
        {
            if constexpr (I == 0)
            {
                return std::move(r.first);
            }
            else
            {
                return std::move(r.second);
            }
        }

        template<std::size_t I>
        static auto&& get(const std::pair<T, U>& r)
        {
            if constexpr (I == 0)
            {
                return r.first;
            }
            else
            {
                return r.second;
            }
        }
    };

    template<typename T, std::size_t N>
    struct range_traits<std::array<T, N>> : std::integral_constant<std::size_t, N>
    {
        template<std::size_t I>
        static auto&& get(std::array<T, N>& r)
        {
            if (I < N) return r[I];
        }

        template<std::size_t I>
        static auto&& get(std::array<T, N>&& r)
        {
            if (I < N) return std::move(r[I]);
        }

        template<std::size_t I>
        static auto&& get(const std::array<T, N>& r)
        {
            if (I < N) return r[I];
        }
    };

    template<typename ... Pack>
    struct range_traits<std::tuple<Pack ...>> : std::integral_constant<std::size_t, sizeof...(Pack)>
    {
        template<std::size_t I>
        static auto&& get(std::tuple<Pack ...>& r)
        {
            return std::get<I>(r);
        }

        template<std::size_t I>
        static auto&& get(std::tuple<Pack ...>&& r)
        {
            return std::move(std::get<I>(r));
        }

        template<std::size_t I>
        static auto&& get(const std::tuple<Pack ...>& r)
        {
            return std::get<I>(r);
        }
    };
}

namespace static_views = static_ranges::views;

#endif //_STATIC_RANGES_H