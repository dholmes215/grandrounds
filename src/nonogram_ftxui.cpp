//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "nonogram_ftxui.hpp"
#include "grid.hpp"

#include <fmt/format.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <gsl/narrow>

namespace grandrounds {

void draw_photo_on_canvas(ftxui::Canvas& canvas,
                          loaded_image& photo,
                          canvas_coords offset)
{
    // Offset must be a terminal character; mask out last bits to make x
    // multiple of 2 and y multiple of 4
    offset.x &= -1;
    offset.y &= -3;

    auto pixel_colors{photo.rgba_pixel_data | rv::chunk(4) |
                      rv::transform([](auto&& pixel) {
                          return ftxui::Color{pixel[0], pixel[1], pixel[2]};
                      })};
    const int width{gsl::narrow<int>(photo.width)};
    const int height{gsl::narrow<int>(photo.height)};
    auto color_rows{grid_rows(pixel_colors, width)};
    for (int y{0}; y < height; y += 2) {
        for (int x{0}; x < width; x++) {
            const std::string c{"▄"};
            const std::function stylizer{[=](ftxui::Pixel& p) {
                p.background_color = color_rows[y][x];
                p.foreground_color = color_rows[y + 1][x];
            }};
            canvas.DrawText(x * 2 + offset.x, y * 2 + offset.y, c, stylizer);
        }
    }
}

namespace {

// These are functions instead of static constants because somewhat
// surprisingly, the ftxui::Color constructor makes different colors at
// runtime based on environment variables, and can throw exceptions.
// clang-format off
[[nodiscard]] ftxui::Color black() { return {0, 0, 0}; } // NOLINT magic numbers - these are for all intents and purposes defining constants.
[[nodiscard]] ftxui::Color almost_black() { return {32, 32, 32}; } // NOLINT magic numbers
[[nodiscard]] ftxui::Color black_select() { return {32, 32, 64}; } // NOLINT magic numbers
[[nodiscard]] ftxui::Color white() { return {255, 255, 255}; } // NOLINT magic numbers
[[nodiscard]] ftxui::Color white_select() { return {223, 223, 255}; } // NOLINT magic numbers
[[nodiscard]] ftxui::Color gray() { return {128, 128, 128}; } // NOLINT magic numbers
[[nodiscard]] ftxui::Color gray_select() { return {128, 128, 160}; } // NOLINT magic numbers
[[nodiscard]] ftxui::Color red() { return {255, 0, 0}; } // NOLINT magic numbers
// clang-format on

}  // namespace

nonogram_component::nonogram_component(std::shared_ptr<nonogram_game> game)
    : game_{std::move(game)},
      board_position_{game_->puzzle->row_hints_max * 3 + 1,
                      game_->puzzle->col_hints_max + 1}
{
}

ftxui::Element nonogram_component::Render()
{
    return ftxui::canvas(solved_ ? draw_photo() : draw_board());
}

bool nonogram_component::OnEvent(ftxui::Event event)
{
    const auto& puzzle{*game_->puzzle};
    const int width{puzzle.dimensions.x};
    const int height{puzzle.dimensions.y};
    if (event.is_mouse()) {
        const int mouse_x = event.mouse().x;
        const int mouse_y = event.mouse().y;
        selected_ = {(mouse_x - board_position_.x) / 2,
                     mouse_y - board_position_.y};
        bool in_range{selected_.x >= 0 && selected_.x < width &&
                      selected_.y >= 0 && selected_.y < height};
        if (in_range && !solved_) {
            if (event.mouse().motion == ftxui::Mouse::Pressed) {
                const auto board_idx{static_cast<std::size_t>(
                    selected_.y * width + selected_.x)};
                if (event.mouse().button == ftxui::Mouse::Left) {
                    game_->board[board_idx] = board_cell::filled;
                }
                else if (event.mouse().button == ftxui::Mouse::Right) {
                    game_->board[board_idx] = board_cell::clear;
                }
                else if (event.mouse().button == ftxui::Mouse::Middle) {
                    game_->board[board_idx] = board_cell::marked;
                }

                solved_ = check_solution(*game_);
            }
        }
        else {
            selected_ = {-1, -1};
        }
    }

    return false;
}

void nonogram_component::Solve()
{
    game_->board = game_->puzzle->solution;
}

void nonogram_component::Reset()
{
    r::fill(game_->board, board_cell::clear);
    solved_ = false;
}

void nonogram_component::draw_rect(ftxui::Canvas& canvas,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   bool value,
                                   ftxui::Color color)
{
    for (auto [x_, y_] : rv::cartesian_product(rv::ints(x, x + width),
                                               rv::ints(y, y + height))) {
        canvas.DrawBlock(x_, y_, value, color);
    }
}

[[nodiscard]] ftxui::Color nonogram_component::square_color(
    board_coords square) const noexcept
{
    const auto& board{game_->board};
    const int width{game_->puzzle->dimensions.x};
    const auto cell{
        board[gsl::narrow<std::size_t>(square.y * width + square.x)]};
    const bool is_selected{selected_.x == square.x || selected_.y == square.y};
    switch (cell) {
        case board_cell::clear:
            return is_selected ? white_select() : white();
        case board_cell::filled:
            return is_selected ? black_select() : almost_black();
        case board_cell::marked:
            return is_selected ? gray_select() : gray();
        default:
            return red();
    }
}

[[nodiscard]] auto all_points(const auto& dimensions)
{
    return rv::cartesian_product(rv::ints(0, dimensions.y),
                                 rv::ints(0, dimensions.x)) |
           rv::transform([](auto coords) {
               // Swap the coordinates so that y is first in the tuple
               return std::make_tuple(std::get<1>(coords), std::get<0>(coords));
           });
}

canvas_coords term2canvas(term_coords board_position, term_coords term) noexcept
{
    return {(board_position.x + term.x - 1) * 2,
            (board_position.y + term.y - 1) * 4};
}

[[nodiscard]] ftxui::Canvas nonogram_component::draw_photo() const
{
    // const auto& puzzle{*game_->puzzle};
    const int width{game_->puzzle->dimensions.x};
    const int height{game_->puzzle->dimensions.y};

    ftxui::Canvas out{(width + board_position_.x) * 4,
                      (height + board_position_.y) * 4};

    // Draw photo
    for (const auto [x, y] : all_points(game_->puzzle->photo_dimensions)) {
        // TODO: extract function to convert coordinates
        const canvas_coords canvas_offset{term2canvas(board_position_, {0, 0})};
        const color c{game_->puzzle->photo[gsl::narrow<std::size_t>(
            y * game_->puzzle->photo_dimensions.x + x)]};
        out.DrawBlockLine(canvas_offset.x + x * 2, canvas_offset.y + y * 2,
                          canvas_offset.x + x * 2 + 1, canvas_offset.y + y * 2,
                          {c.r, c.g, c.b});
    }
    return out;
}

[[nodiscard]] ftxui::Canvas nonogram_component::draw_board() const
{
    const auto& puzzle{*game_->puzzle};
    const int width{game_->puzzle->dimensions.x};
    const int height{game_->puzzle->dimensions.y};

    ftxui::Canvas out{(width + board_position_.x) * 4,
                      (height + board_position_.y) * 4};

    // Draw board
    for (const auto [x, y] : all_points(game_->puzzle->dimensions)) {
        // TODO: extract function to convert coordinates
        draw_rect(out, 4 * x + 2 * board_position_.x,
                  4 * (y + board_position_.y), 4, 4, true,
                  square_color({x, y}));
    }

    const std::function default_stylizer{[=](ftxui::Pixel& p) {
        p.background_color = black();
        p.foreground_color = white();
    }};

    const std::function highlight_stylizer{[=](ftxui::Pixel& p) {
        p.background_color = white_select();
        p.foreground_color = black();
    }};

    // Draw row hints
    for (int y{0}; y < height; y++) {
        const auto& this_row_hints{
            puzzle.row_hints[gsl::narrow<std::size_t>(y)]};
        const auto canvas_y{(board_position_.y + y) * 4};
        for (const auto [i, hint] :
             this_row_hints | rv::reverse | rv::enumerate) {
            const auto str{fmt::format("{:4}", hint)};
            const auto canvas_x{
                (board_position_.x - (3 * (gsl::narrow<int>(i) + 1)) - 1) * 2};
            const auto& stylizer{selected_.y == y ? highlight_stylizer
                                                  : default_stylizer};
            out.DrawText(canvas_x, canvas_y, str, stylizer);
        }
    }

    // Draw column hints
    for (int x{0}; x < width; x++) {
        const auto& this_col_hints{
            puzzle.col_hints[gsl::narrow<std::size_t>(x)]};
        const auto canvas_x{(board_position_.x + x * 2) * 2};
        for (auto [i, hint] : this_col_hints | rv::reverse | rv::enumerate) {
            const auto str{fmt::format("{:2}", hint)};
            const auto canvas_y{
                (board_position_.y - (gsl::narrow<int>(i) + 1)) * 4};
            const auto& stylizer{selected_.x == x ? highlight_stylizer
                                                  : default_stylizer};
            out.DrawText(canvas_x, canvas_y, str, stylizer);
        }
    }

    return out;
}

}  // namespace grandrounds
