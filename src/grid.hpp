//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef GRID_HPP
#define GRID_HPP

#include "range.hpp"

namespace grandrounds {

template <typename Range>
auto grid_rows(Range& r, int width) noexcept
{
    return r | rv::chunk(width);
}

template <typename Range>
auto grid_row(Range& r, int width, int row) noexcept
{
    return grid_rows(r, width)[row];
}

template <typename Range>
auto grid_col(Range& r, int width, int col) noexcept
{
    return r | rv::drop(col) | rv::stride(width);
}

template <typename Range>
auto grid_cols(Range& r, int width) noexcept
{
    return rv::ints(0, width) |
           rv::transform([&r, width](int i) { return grid_col(r, width, i); });
}

}  // namespace grandrounds

#endif  // GRID_HPP
