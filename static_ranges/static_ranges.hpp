#ifndef _STATIC_RANGES_H
#define _STATIC_RANGES_H

#include <utility>
#include <tuple>
#include <array>
#include <cstdint>
#include <type_traits>
#include <cassert>
#include <iostream>

namespace static_ranges
{
    // Specializations are used to satisfy the static_ranges::range concept
    template <typename T> struct range_traits;
    // An empty struct 
    // Inherited to denote static ranges that satisfy the static_ranges::view concept
    struct view_base {};

    // Concepts
    template <typename, typename = void>
    static constexpr bool is_complete_v = false;

    template <typename T>
    static constexpr bool is_complete_v<T, std::void_t<decltype(sizeof(T))> > = true;

    template <typename T>
    concept range = requires(T r)
    {
        is_complete_v<range_traits<std::remove_cvref_t<T> > >;
        std::is_same_v<decltype(static_ranges::range_traits<std::remove_cvref_t<T> >::value), std::size_t>;
    };

    template <typename T>
    concept view = range<T> &&std::is_base_of_v<view_base, T>;

    // Convenience
    template <typename T>
    static constexpr std::size_t size_v = static_cast<std::size_t>(range_traits<std::remove_cvref_t<T>>::value);

    template <typename T, typename U>
    static constexpr std::size_t size_v<std::pair<T, U>> = 2;

    template <typename... Pack>
    static constexpr std::size_t size_v<std::tuple<Pack...> > = sizeof...(Pack);

    template <std::size_t Size, typename T>
    static constexpr std::size_t size_v<std::array<T, Size> > = Size;

    template <std::size_t I, range R>
    inline constexpr decltype(auto) element(R && r)
    {   
        return range_traits<std::remove_cvref_t<R>>::template get<I>(std::forward<R>(r));
    }

    // static_ranges::copy
    // copies the static range r1 into the static range r2
    template <std::size_t I, std::size_t Size>
    constexpr void copy_impl(range auto &&r1, range auto &&r2)
    {
        if constexpr (I < Size)
        {
            element<I>(r2) = element<I>(r1);
            copy_impl<I + 1, Size>(r1, r2);
        }
    }

    constexpr void copy(range auto &&r1, range auto &&r2)
    {
        copy_impl<0, size_v<std::remove_cvref_t<decltype(r1)> > >(std::forward<decltype(r1)>(r1), std::forward<decltype(r2)>(r2));
    }

    // for_each
    // applies f(e) to every element e of the static range r
    template <std::size_t I, std::size_t Size>
    constexpr void for_each_impl(range auto &&r, auto f)
    {
        if constexpr (I < Size)
        {
            f(element<I>(r));
            for_each_impl<I + 1, Size>(std::forward<decltype(r)>(r), f);
        }
    }

    constexpr void for_each(range auto &&r, auto f)
    {
        for_each_impl<0, size_v<std::remove_cvref_t<decltype(r)> > >(std::forward<decltype(r)>(r), f);
    }

    // static_views 
    namespace views
    {
        // all_view
        template <range R>
        struct all_view : view_base
        {
            all_view(R &&r) : r(std::forward<R>(r)) {}
            operator R &() { return r; }
            R &r;
        };

        template <typename RV>
        auto all(RV &rv)
        {
            if constexpr (std::is_base_of_v<view_base, RV>)
            {
                return all_view<RV>(rv);
            }
            else
            {
                return all_view<RV &>(rv);
            }
        };

        // iota_view
        template<typename T, std::size_t Size>
        struct iota_view : view_base {};

        template <typename T, std::size_t Size>
        auto iota()
        {
            return iota_view<T, Size>();
        }

        // static_iota_view
        template <typename T, std::size_t Size>
        struct static_iota_view : view_base {};

        template <typename T, std::size_t Size>
        auto static_iota()
        {
            static_assert(std::is_integral_v<T>);
            return static_iota_view<T, Size>();
        }

        // operator | for transform_view
        template <typename R, typename C>
        auto operator|(R &&r, C &&c)
        {
            return c(std::forward<R>(r));
        }

        // transform_view
        template <typename R, typename F>
        struct transform_view : view_base
        {
            transform_view(R &&r, F &&f) : r(std::forward<R>(r)), f(std::forward<F>(f)) {}
            template <std::size_t I>
            decltype(auto) get()
            {
                return c(r.template get<I>());
            }
            R&& r;
            F&& f;
        };

        template <typename R, typename F>
        auto transform(R &&r, F &&f)
        {
            return transform_view<R, F>(std::forward<R>(r), std::forward<F>(f));
        }

        template<typename F>
        struct transform_view<void, F> : view_base
        {
            transform_view(F &&f) : f(std::forward<F>(f)) {}
            template <std::size_t I>
            decltype(auto) get()
            {
                return c(f(I));
            }
            F&& f;
        };

        //  transform without range returns a lambda which, when given range, creates transform_view
        template <typename F>
        auto transform(F &&f)
        {
            return [f = std::forward<F>(f)](auto &&r)
            {         
                return transform_view<decltype(r), decltype(f)>(std::forward<decltype(r)>(r), std::forward<decltype(f)>(f));
            };
        }
    }

    template <typename T, std::size_t Size>
    struct range_traits<std::array<T, Size> > : std::integral_constant<std::size_t, Size>
    {
        template <std::size_t I>
        static auto &&get(std::array<T, Size> &r)
        {
            if (I < Size)
            {
                return r[I];
            }
        }

        template <std::size_t I>
        static auto &&get(std::array<T, Size> &&r)
        {
            if (I < Size)
            {
                return std::move(r[I]);
            }
        }

        template <std::size_t I>
        static auto &&get(const std::array<T, Size> &r)
        {
            if (I < Size)
            {
                return r[I];
            }
        }
    };

    template <typename... Pack>
    struct range_traits<std::tuple<Pack...> > : std::integral_constant<std::size_t, sizeof...(Pack)>
    {
        template <std::size_t I>
        static auto &&get(std::tuple<Pack...> &r)
        {
            return std::get<I>(r);
        }

        template <std::size_t I>
        static auto &&get(std::tuple<Pack...> &&r)
        {
            return std::move(std::get<I>(r));
        }

        template <std::size_t I>
        static auto &&get(const std::tuple<Pack...> &r)
        {
            return std::get<I>(r);
        }
    };

    // to_tuple
    template <std::size_t I, std::size_t Size>
    constexpr auto to_tuple_impl(range auto &&r)
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

    constexpr auto to_tuple(range auto &&r)
    {
        return to_tuple_impl<0, size_v<std::remove_cvref_t<decltype(r)> > >(std::forward<decltype(r)>(r));
    }

    // to_pair
    template <typename T>
    constexpr auto to_pair(T &&r)
    {
        // r must have exactly 2 elements
        assert(size_v<T> == 2);
        return std::pair<decltype(element<0>(r)), decltype(element<1>(r))>(element<0>(r), element<1>(r));
    }

    // to_array
    template <typename T, range R>
    inline constexpr auto to_array(R &&r)
    {
        std::array<T, size_v<std::remove_cvref_t<R> > > arr;
        copy(r, arr);
        return arr;
    }

    // std
    template <typename T, typename U>
    struct range_traits<std::pair<T, U> > : std::integral_constant<std::size_t, 2>
    {
        template <std::size_t I>
        static auto &&get(std::pair<T, U> &r)
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

        template <std::size_t I>
        static auto &&get(std::pair<T, U> &&r)
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

        template <std::size_t I>
        static auto &&get(const std::pair<T, U> &r)
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

    // transform(r1,r2,f);
    // copies the value f(e1I) for every element e1I of the static range r1 into the corresponding element e2I of the static range r2
    template <std::size_t I, std::size_t Size, range R1, range R2, typename F>
    constexpr void transform_impl(R1 &&r1, R2 &&r2, F &&f)
    {
        if constexpr (I < Size)
        {
            element<I>(r2) = f(element<I>(r1));
            transform_impl<I + 1, Size>(std::forward<R1>(r1), std::forward<R2>(r2), std::forward<F>(f));
        }
    }

    // transform(r1,r2,r3,f);
    // copies the value f(e1I,e2I) for every element e1I of the static range r1 and the corresponding element e2I of the static range r2 into the corresponding element e3I of the static range r3
    template <std::size_t I, std::size_t Size, range R1, range R2, range R3, typename F>
    constexpr void transform_impl(R1 &&r1, R2 &&r2, R3 &&r3, F &&f)
    {
        if constexpr (I < Size)
        {
            element<I>(r3) = f(element<I>(r1), element<I>(r2));
            transform_impl<I + 1, Size>(std::forward<R1>(r1), std::forward<R2>(r2), std::forward<R3>(r3), std::forward<F>(f));
        }
    }

    // static_ranges::transform(r1,r2,f);
    template <typename R1, typename R2, typename F>
    constexpr auto transform(R1 &&r1, R2 &&r2, F &&f)
    {
        static_assert(size_v<R1> == size_v<R2>);
        transform_impl<0, size_v<R1>, R1, R2, F>(std::forward<R1>(r1), std::forward<R2>(r2), std::forward<F>(f));
    }

    // static_ranges::transform(r1,r2,r3,f);
    template <typename R1, typename R2, typename R3, typename F>
    constexpr auto transform(R1 &&r1, R2 &&r2, R3 &&r3, F &&f)
    {
        static_assert(size_v<R1> == size_v<R2>);
        static_assert(size_v<R1> == size_v<R3>);
        transform_impl<0, size_v<R1>, R1, R2, R3, F>(std::forward<R1>(r1), std::forward<R2>(r2), std::forward<R3>(r3), std::forward<F>(f));
    }

    // view traits
    template <range R>
    struct range_traits<views::all_view<R> > : std::integral_constant<std::size_t, size_v<R> >
    {
        template <std::size_t I>
        static auto &&get(views::all_view<R> &v)
        {
            return element<I>(v.r);
        }
    };

    // range trait of iota view
    template<typename T, std::size_t N>
    struct range_traits<views::iota_view<T, N>> : std::integral_constant<std::size_t, N>
    {
        template<std::size_t I>
        static auto get(views::iota_view<T, N>)
        {
            return static_cast<T>(I);
        }
    };

    template <typename T, std::size_t Size>
    struct range_traits<views::static_iota_view<T, Size> > : std::integral_constant<std::size_t, Size>
    {
        template <std::size_t I>
        static auto get(views::static_iota_view<T, Size>)
        {
            return std::integral_constant<T, I>();
        }
    };

    // range trait for transform view
    template <typename R, typename F>
    struct range_traits<views::transform_view<R, F> > : std::integral_constant<std::size_t, size_v<R> >
    {
        template <std::size_t I>
        static auto get(views::transform_view<R, F> &v)
        {
            return v.f(element<I>(v.r));
        }
    };

    // transform without range 
    template <typename F>
    struct range_traits<views::transform_view<void, F> > : std::integral_constant<std::size_t, 0>
    {
        template <std::size_t I>
        static auto get(views::transform_view<void, F> &v)
        {
            return v.f();
        }
    };
}

namespace static_views = static_ranges::views;

#endif