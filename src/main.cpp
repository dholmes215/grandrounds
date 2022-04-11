//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "file.hpp"
#include "grid.hpp"
#include "nonogram.hpp"
#include "nonogram_ftxui.hpp"
#include "range.hpp"

#include <fmt/format.h>
#include <ftxui/component/captured_mouse.hpp>      // for ftxui
#include <ftxui/component/component.hpp>           // for Slider
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive

#include <gsl/narrow>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string_view>
#include <vector>

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `grandrounds`. You can
// modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

namespace grandrounds {

namespace {

void play_puzzle(ftxui::ScreenInteractive& screen, std::string_view name)
{
    int mouse_x{0};
    int mouse_y{0};
    int frame{0};

    auto game{std::make_shared<nonogram_game>()};
    // TODO: Move this to nonogram_game constructor or static factory or
    // something
    game->puzzle = std::make_shared<nonogram_puzzle>(name);
    game->board.resize(game->puzzle->solution.size());

    const std::string solve_text{"Solve"};
    const std::string reset_text{"Reset"};
    const std::string quit_text{"Quit"};

    auto puzzle_component{std::make_shared<nonogram_component>(game)};
    auto solve_button{
        ftxui::Button(&solve_text, [&] { puzzle_component->Solve(); })};
    auto reset_button{
        ftxui::Button(&reset_text, [&] { puzzle_component->Reset(); })};
    auto quit_button{ftxui::Button(&quit_text, screen.ExitLoopClosure())};
    auto right_container{
        ftxui::Container::Vertical({solve_button, reset_button, quit_button})};

    std::vector<ftxui::Component> all_components;
    all_components.push_back(puzzle_component);

    auto right_panel{ftxui::Renderer(right_container, [&] {
        return ftxui::vbox(
            {{ftxui::text(fmt::format("Mouse: {},{}", mouse_x, mouse_y)),
              ftxui::text(fmt::format("Frame: {}", frame++)),
              solve_button->Render(), reset_button->Render(),
              quit_button->Render()}});
    })};
    all_components.push_back(right_panel);

    auto container = ftxui::Container::Horizontal(all_components);
    auto container_with_mouse =
        ftxui::CatchEvent(container, [&](ftxui::Event e) {
            if (e.is_mouse()) {
                mouse_x = (e.mouse().x - 1) * 2;
                mouse_y = (e.mouse().y - 1) * 4;
            }
            return false;
        });

    screen.Loop(container_with_mouse);
}

void play_puzzles(ftxui::ScreenInteractive& screen)
{
    play_puzzle(screen, "cottontail");
    play_puzzle(screen, "lake_mendoza");
}

loaded_image load_title_image()
{
    return load_image(find_puzzles_dir() / "title.png");
}

void play_game()
{
    auto screen{ftxui::ScreenInteractive::TerminalOutput()};
    ftxui::Canvas canvas{160, 96};
    auto title_image{load_title_image()};
    draw_photo_on_canvas(canvas, title_image, {2, 2});

    auto start_button{ftxui::Button("Start", [&] { play_puzzles(screen); })};
    auto quit_button{ftxui::Button("Quit", screen.ExitLoopClosure())};
    auto button_container{
        ftxui::Container::Vertical({start_button, quit_button})};

    auto caption{ftxui::vbox(
        {ftxui::paragraphAlignCenter(
             "The Grand Rounds Scenic Byway is a 50-mile loop of parks "
             "and trails around"),
         ftxui::paragraphAlignCenter(
             "    the city of Minneapolis, MN.  Explore by solving "
             "nonogram puzzles!    "),
         ftxui::paragraphAlignCenter(
             "Photo (C) 2005 Adam Backstrom (from "
             "Wikipedia, CC-BA-SA-3.0/GFDL license)")})};

    auto renderer{ftxui::Renderer(button_container, [&] {
        auto layout{ftxui::hbox({ftxui::vbox({ftxui::canvas(canvas), caption}),
                                 button_container->Render()})};
        return layout;
    })};

    screen.Loop(renderer);
}

}  // namespace

}  // namespace grandrounds

int main(int argc, const char** argv)
{
    try {
        const std::span args{argv, gsl::narrow<std::size_t>(argc)};
        static constexpr auto USAGE =
            R"(grandrounds

    Usage:
          grandrounds
          grandrounds puzzle <NAME>
)";
        // XXX I removed docopt because the the current Conan+CMake build
        // intermittently fails to find it.  This is a workaround.
        if (argc == 1) {
            grandrounds::play_game();
        }
        else if (argc == 3 && args[1] == std::string_view{"puzzle"}) {
            auto screen{ftxui::ScreenInteractive::TerminalOutput()};
            grandrounds::play_puzzle(screen, args[2]);
        }
        else {
            fmt::print("{}", USAGE);
        }
    }
    catch (const std::exception& e) {
        fmt::print("Unhandled exception in main: {}", e.what());
    }
}
