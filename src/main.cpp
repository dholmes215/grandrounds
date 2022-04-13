//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "game.hpp"

#include <fmt/format.h>
#include <gsl/narrow>

#include <span>
#include <stdexcept>

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `grandrounds`. You can
// modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

int main(int argc, const char** argv)
{
    try {
        const std::span args{argv, gsl::narrow<std::size_t>(argc)};
        static constexpr auto USAGE =
            R"(grandrounds

    Usage:
          grandrounds
          grandrounds puzzle <NAME>
 Options:
          -h --help     Show this screen.
          --version     Show version.
)";
        // XXX I removed docopt because the the current Conan+CMake build
        // intermittently fails to find it.  This is a workaround.
        if (argc == 1) {
            grandrounds::play_game();
        }
        else if (argc == 3 && args[1] == std::string_view{"puzzle"}) {
            grandrounds::play_puzzle(args[2]);
        }
        else if (args[1] == std::string_view{"--version"}) {
            fmt::print("{} {}", grandrounds::cmake::project_name,
                       grandrounds::cmake::project_version);
        }
        else {
            fmt::print("{}", USAGE);
        }
    }
    catch (const std::exception& e) {
        fmt::print("Unhandled exception in main: {}", e.what());
    }
}
