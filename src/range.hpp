//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RANGE_HPP
#define RANGE_HPP

// Range-v3 has some headers that generate warnings in MSVC.
// TODO: Document in more detail
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242)
#endif
#include <range/v3/all.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif 

namespace grandrounds {

namespace r = ranges;
namespace rv = ranges::views;
namespace ra = ranges::actions;

}  // namespace grandrounds

#endif  // RANGE_HPP
