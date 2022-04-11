//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "file.hpp"
#include "nonogram.hpp"
#include "grid.hpp"
#include "range.hpp"

#include <fmt/format.h>
#include <lodepng.h>
#include <gsl/narrow>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace grandrounds {

namespace {

// Read an entire file into a std::string.  Will throw if any failure occurs.
// TODO: test
std::string slurp(std::istream& stream)
{
    std::stringstream buffer;
    buffer << stream.rdbuf();
    if (stream.fail()) {
        throw path_error{"Could not read file"};
    }
    return buffer.str();
}

// Read an entire file into a std::string.  Will throw if any failure occurs.
// TODO: test
std::string slurp(const std::filesystem::path& path)
{
    std::ifstream stream{path};
    if (!stream) {
        throw path_error{"Could not open file: " + path.string()};
    }
    return slurp(stream);
}

puzzle_data load_puzzle_data(const std::filesystem::path& json_path)
{
    const auto json_text{slurp(json_path)};
    const auto parsed_json{nlohmann::json::parse(json_text)};

    puzzle_data out;
    out.title = parsed_json["title"];
    out.description = parsed_json["description"];
    out.author = parsed_json["author"];
    out.date = parsed_json["date"];
    out.license = parsed_json["license"];
    out.wikipedia = parsed_json["wikipedia"];

    return out;
}


std::vector<std::uint8_t> calculate_hints(const auto& row_or_column)
{
    auto iter{row_or_column.begin()};
    const auto end{row_or_column.end()};
    std::vector<std::uint8_t> out;
    while (iter != end) {
        while (iter != end && *iter == board_cell::clear) {
            ++iter;
        }
        std::uint8_t count{0};
        while (iter != end && *iter == board_cell::filled) {
            ++count;
            ++iter;
        }
        if (count > 0) {
            out.push_back(count);
        }
    }
    // TODO there may be some marvelous way to replace all this with ranges,
    // using group_by or something
    return out;
}

}  // namespace

nonogram_puzzle::nonogram_puzzle(std::string_view name)
{
    const auto puzzle_dir{find_puzzles_dir()};
    const auto json_path{puzzle_dir / fmt::format("{}_data.json", name)};
    const auto nonogram_path{puzzle_dir / fmt::format("{}_nonogram.png", name)};
    const auto photo_path{puzzle_dir / fmt::format("{}_photo.png", name)};
    auto solution_image{load_image(nonogram_path)};
    auto photo_image{load_image(photo_path)};

    dimensions.x = gsl::narrow<int>(solution_image.width);
    dimensions.y = gsl::narrow<int>(solution_image.height);
	photo_dimensions.x = gsl::narrow<int>(photo_image.width);
	photo_dimensions.y = gsl::narrow<int>(photo_image.height);
    // Split image data into four-byte (RGBA) chunks and convert those to board
    // cells
    solution = solution_image.rgba_pixel_data | rv::chunk(4) |
               rv::transform([](auto&& pixel) {
                   const bool filled{(pixel[0] == 0) && (pixel[1] == 0) &&
                                     (pixel[2] == 0)};
                   return filled ? board_cell::filled : board_cell::clear;
               }) |
               r::to<std::vector>;

    photo = photo_image.rgba_pixel_data | rv::chunk(4) |
            rv::transform([](auto&& pixel) -> color {
                return {pixel[0], pixel[1], pixel[2]};
            }) |
            r::to<std::vector>;

    data = load_puzzle_data(json_path);

    const auto cols{grid_cols(solution, dimensions.x)};
    col_hints =
        cols |
        rv::transform([&](const auto& col) { return calculate_hints(col); }) |
        r::to<std::vector>;

    const auto rows{grid_rows(solution, dimensions.x)};
    row_hints =
        rows |
        rv::transform([&](const auto& row) { return calculate_hints(row); }) |
        r::to<std::vector>;

    const auto vec_size{[](const std::vector<std::uint8_t>& vec) {
        return static_cast<int>(vec.size());
    }};
    row_hints_max = r::max(row_hints | rv::transform(vec_size));
    col_hints_max = r::max(col_hints | rv::transform(vec_size));
}

bool check_solution(const nonogram_game& game) noexcept
{
    // Filter out "marked" cells so we can compare directly with the solution.
    auto board_filled_cells{game.board | rv::transform([](auto&& cell) {
                                return cell == board_cell::filled
                                           ? board_cell::filled
                                           : board_cell::clear;
                            })};
    return r::equal(board_filled_cells, game.puzzle->solution);
}

}  // namespace grandrounds
