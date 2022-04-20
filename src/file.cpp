//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "file.hpp"

#include <fmt/format.h>
#include <lodepng.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace grandrounds {

path_error::path_error(const std::string& message) : std::runtime_error(message)
{
}

file_error::file_error(const std::string& message) : std::runtime_error(message)
{
}

// Read an entire file into a std::string.  Will throw if any failure occurs.
std::string slurp(std::istream& stream)
{
    std::stringstream buffer;
    buffer << stream.rdbuf();
    if (stream.fail()) {
        throw file_error{"Could not read file"};
    }
    return buffer.str();
}

// Read an entire file into a std::string.  Will throw if any failure occurs.
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

}  // namespace grandrounds
