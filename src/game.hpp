//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef GAME_HPP
#define GAME_HPP

#include <string_view>

namespace grandrounds {

void play_puzzle(std::string_view name);
void play_game();

}  // namespace grandrounds

#endif  // GAME_HPP
