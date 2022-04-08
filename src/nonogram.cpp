//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "grid.hpp"
#include "nonogram.hpp"
#include "range.hpp"

#include <fmt/format.h>
#include <gsl/narrow>
#include <lodepng.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace grandrounds {

namespace {

class path_error : public std::runtime_error {
   public:
    explicit path_error(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

class file_error : public std::runtime_error {
   public:
    explicit file_error(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

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

// Auto-detect the directory containing puzzle files.
std::filesystem::path find_puzzles_dir()
{
    auto path = std::filesystem::current_path();
    while (!std::filesystem::exists(path / "puzzles")) {
        auto last_path_size = path.string().size();
        path = path.parent_path();
        if (path.string().size() >= last_path_size) {
            throw grandrounds::path_error("Could not locate puzzles directory");
        }
    }
    return std::filesystem::canonical(path / "puzzles");
}

struct loaded_image {
    std::vector<std::uint8_t> rgba_pixel_data;
    unsigned int width{};
    unsigned int height{};
};

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

loaded_image load_image(const std::filesystem::path& nonogram_png_path)
{
    loaded_image out;
    const auto error{lodepng::decode(out.rgba_pixel_data, out.width, out.height,
                                     nonogram_png_path.string())};
    if (error != 0) {
        throw file_error{fmt::format("Could not load {}: {} {}",
                                     nonogram_png_path.string(), error,
                                     lodepng_error_text(error))};
    }
    return out;
}

std::vector<std::uint8_t> calculate_hints(const auto& row_or_column)
{
    auto iter{row_or_column.begin()};
    const auto end{row_or_column.end()};
    std::vector<std::uint8_t> out;
    while (iter != end) {
        while (iter != end && *iter == 0) {
            ++iter;
        }
        std::uint8_t count{0};
        while (iter != end && *iter == 1) {
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
    auto loaded_image{load_image(nonogram_path)};

    // Convert the image data to "white = 0, black = 1"
    dimensions.x = gsl::narrow<int>(loaded_image.width);
    dimensions.y = gsl::narrow<int>(loaded_image.height);
    nonogram = loaded_image.rgba_pixel_data | rv::chunk(4) |
               rv::transform([](auto&& pixel) -> std::uint8_t {
                   return (pixel[0] == 0) && (pixel[1] == 0) && (pixel[2] == 0);
               }) |
               r::to<std::vector>;
    data = load_puzzle_data(json_path);

    const auto cols{grid_cols(nonogram, dimensions.x)};
    col_hints =
        cols |
        rv::transform([&](const auto& col) { return calculate_hints(col); }) |
        r::to<std::vector>;

    const auto rows{grid_rows(nonogram, dimensions.x)};
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

}  // namespace grandrounds
