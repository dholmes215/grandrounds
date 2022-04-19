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
#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `grandrounds`. You can
// modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

namespace grandrounds {

namespace {

void show_info(ftxui::ScreenInteractive& screen, nonogram_game& game)
{
    auto& photo{game.puzzle->photo};
    int width{gsl::narrow<int>(photo.width)};
    int height{gsl::narrow<int>(photo.height)};
    ftxui::Canvas canvas{width * 2, height * 2};
    draw_photo_on_canvas(canvas, photo, {0, 0});

    auto continue_button{ftxui::Button("Continue", screen.ExitLoopClosure())};

    auto button_with_info{ftxui::Renderer(continue_button, [&] {
        return ftxui::hbox(
            {ftxui::canvas(canvas),
             ftxui::vbox(
                 {ftxui::text(game.puzzle->data.title),
                  ftxui::paragraph(game.puzzle->data.description),
                  ftxui::text(fmt::format("{}, {}", game.puzzle->data.author,
                                          game.puzzle->data.date)),
                  ftxui::text(game.puzzle->data.license),
                  continue_button->Render()})});
    })};

    screen.Loop(button_with_info);
}

void play_puzzle(ftxui::ScreenInteractive& screen, std::string_view name)
{
    auto game{std::make_shared<nonogram_game>()};
    game->puzzle = std::make_shared<nonogram_puzzle>(name);
    game->board.resize(game->puzzle->solution.size());

    const std::string solve_text{"Solve"};
    const std::string reset_text{"Reset"};
    std::string quit_continue_text{"Quit"};

    auto puzzle_component{std::make_shared<nonogram_component>(game)};
    auto solve_button{
        ftxui::Button(&solve_text, [&] { puzzle_component->Solve(); })};
    auto reset_button{
        ftxui::Button(&reset_text, [&] { puzzle_component->Reset(); })};

    auto quit_button{
        ftxui::Button(&quit_continue_text, screen.ExitLoopClosure())};
    auto right_container{
        ftxui::Container::Vertical({solve_button, reset_button, quit_button})};

    std::vector<ftxui::Component> all_components;
    all_components.push_back(puzzle_component);

    auto right_panel{ftxui::Renderer(right_container, [&] {
        return ftxui::vbox(
            {{ftxui::text(fmt::format("Width: {}", game->puzzle->dimensions.x)),
              ftxui::text(
                  fmt::format("Height: {}", game->puzzle->dimensions.y)),
              solve_button->Render(), reset_button->Render(),
              quit_button->Render()}});
    })};
    all_components.push_back(right_panel);

    auto container = ftxui::Container::Horizontal(all_components);

    screen.Loop(container);

    if (puzzle_component->IsSolved()) {
        show_info(screen, *game);
    }
}

}  // namespace

// Suppress cppcheck because passing string_view by value is correct.
// cppcheck-suppress passedByValue
void play_puzzle(std::string_view name)
{
    auto screen{ftxui::ScreenInteractive::TerminalOutput()};
    play_puzzle(screen, name);
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
    ftxui::Canvas canvas{160, 96};  // NOLINT magic number to fit terminal
    auto title_image{load_title_image()};
    draw_photo_on_canvas(canvas, title_image, {0, 0});

    bool start_clicked{false};

    auto start_button{ftxui::Button("Start", [&] {
        start_clicked = true;
        screen.ExitLoopClosure()();
    })};
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

    // This is a cppcheck false positive
    // cppcheck-suppress knownConditionTrueFalse
    if (start_clicked) {
        play_puzzles(screen);
    }
}

}  // namespace grandrounds
