//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NONOGRAM_HPP
#define NONOGRAM_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace grandrounds {

struct puzzle_data {
    std::string title;
    std::string description;
    std::string author;
    std::string date;
    std::string license;
    std::string wikipedia;
};

struct color {
    std::uint8_t r{0};
    std::uint8_t g{0};
    std::uint8_t b{0};
};

enum class board_cell : std::uint8_t { clear, filled, marked };

// The terminal uses a coordinate system where the top-left character is (1,1),
// the next character to the right is (2,1), the next character down is (1,2),
// and so on.
struct term_coords {
    int x{0};
    int y{0};
};

// ftxui::Canvas uses a coordinate system where each character in the terminal
// is subdivided into a 2x8 grid of subpixels, so each character is two
// subpixels wide and one subpixel high.  The canvas is indexed from (0,0),
// unlike the terminal.
struct canvas_coords {
    int x{0};
    int y{0};
};

// The nonogram board is drawn using squares that are two characters wide and
// one character high, which on a canvas is 8x8 subpixels.  The board's top-left
// square is (0,0) but the board is drawn at an offset from the top-left
// terminal character, which can vary from puzzle to puzzle, so that offset
// needs to be added or subtracted when translating between coordinate systems.
struct board_coords {
    int x{0};
    int y{0};
};

struct nonogram_puzzle {
    explicit nonogram_puzzle(std::string_view name);

    board_coords dimensions;
    std::vector<board_cell> solution;
    canvas_coords photo_dimensions;
    std::vector<color> photo;
    puzzle_data data;
    std::vector<std::vector<std::uint8_t>> row_hints;
    std::vector<std::vector<std::uint8_t>> col_hints;
    int row_hints_max{0};
    int col_hints_max{0};
};

struct nonogram_game {
    std::shared_ptr<nonogram_puzzle> puzzle;
    std::vector<board_cell> board;
};

bool check_solution(const nonogram_game& game) noexcept;

}  // namespace grandrounds

#endif  // NONOGRAM_HPP
