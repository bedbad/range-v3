/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//
#ifndef RANGES_V3_UTILITY_COMMON_ITERATOR_HPP
#define RANGES_V3_UTILITY_COMMON_ITERATOR_HPP

#include <type_traits>
#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/view_facade.hpp>
#include <range/v3/utility/basic_iterator.hpp>
#include <range/v3/utility/concepts.hpp>
#include <range/v3/utility/variant.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \cond
        namespace detail
        {
            template<typename I, typename S>
            struct common_cursor
            {
                using single_pass = SinglePass<I>;
                struct mixin
                  : basic_mixin<common_cursor>
                {
                    mixin() = default;
                    mixin(common_cursor pos)
                      : basic_mixin<common_cursor>{std::move(pos)}
                    {}
                    explicit mixin(I it)
                      : mixin(common_cursor{std::move(it)})
                    {}
                    explicit mixin(S se)
                      : mixin(common_cursor{std::move(se)})
                    {}
                };
            private:
                static_assert(!std::is_same<I, S>::value,
                              "Error: iterator and sentinel types are the same");
                variant<I, S> data_;
                bool is_sentinel() const
                {
                    RANGES_ASSERT(data_.valid());
                    return data_.index() == 1u;
                }
                I & it()
                {
                    RANGES_ASSERT(!is_sentinel());
                    return ranges::get<0>(data_);
                }
                I const & it() const
                {
                    RANGES_ASSERT(!is_sentinel());
                    return ranges::get<0>(data_);
                }
                S const & se() const
                {
                    RANGES_ASSERT(is_sentinel());
                    return ranges::get<1>(data_);
                }
                CONCEPT_REQUIRES(SizedIteratorRange<I, S>() && SizedIteratorRange<I, I>())
                friend iterator_difference_t<I>
                operator-(common_iterator<I, S> const &end, common_iterator<I, S> const &begin)
                {
                    common_cursor const &this_ = get_cursor(begin), &that = get_cursor(end);
                    return that.is_sentinel() ?
                        (this_.is_sentinel() ? 0 : that.se() - this_.it()) :
                        (this_.is_sentinel() ?
                             that.it() - this_.se() :
                             that.it() - this_.it());
                }
                // BUGBUG TODO what about advance??
            public:
                common_cursor() = default;
                explicit common_cursor(I it)
                  : data_(emplaced_index<0>, std::move(it))
                {}
                explicit common_cursor(S se)
                  : data_(emplaced_index<1>, std::move(se))
                {}
                template<typename I2, typename S2,
                    CONCEPT_REQUIRES_(ExplicitlyConvertibleTo<I, I2>() &&
                                      ExplicitlyConvertibleTo<S, S2>())>
                operator common_cursor<I2, S2>() const
                {
                    return is_sentinel() ?
                        common_cursor<I2, S2>{S2{se()}} :
                        common_cursor<I2, S2>{I2{it()}};
                }
                auto get() const -> decltype(*std::declval<I const &>())
                {
                    return *it();
                }
                template<typename I2, typename S2,
                    CONCEPT_REQUIRES_(IteratorRange<I, S2>() && IteratorRange<I2, S>() &&
                        !EqualityComparable<I, I2>())>
                bool equal(common_cursor<I2, S2> const &that) const
                {
                    return is_sentinel() ?
                        (that.is_sentinel() || that.it() == se()) :
                        (!that.is_sentinel() || it() == that.se());
                }
                template<typename I2, typename S2,
                    CONCEPT_REQUIRES_(IteratorRange<I, S2>() && IteratorRange<I2, S>() &&
                        EqualityComparable<I, I2>())>
                bool equal(common_cursor<I2, S2> const &that) const
                {
                    return is_sentinel() ?
                        (that.is_sentinel() || that.it() == se()) :
                        (that.is_sentinel() ?
                            it() == that.se() :
                            it() == that.it());
                }
                void next()
                {
                    ++it();
                }
            };
        }
        /// \endcond
    }
}

#endif
