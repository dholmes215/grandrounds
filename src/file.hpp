//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FILE_HPP
#define FILE_HPP

#include <filesystem>
#include <stdexcept>
#include <vector>

namespace grandrounds {

class path_error : public std::runtime_error {
   public:
    explicit path_error(const std::string& message);
};

class file_error : public std::runtime_error {
   public:
    explicit file_error(const std::string& message);
};

struct loaded_image {
    std::vector<std::uint8_t> rgba_pixel_data;
    unsigned int width{};
    unsigned int height{};
};

// Auto-detect the directory containing puzzle files.
std::filesystem::path find_puzzles_dir();

// Load a PNG file and decode it to RGBA pixel data.
loaded_image load_image(const std::filesystem::path& nonogram_png_path);

}  // namespace grandrounds

#endif  // FILE_HPP
