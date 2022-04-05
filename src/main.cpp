//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "grid.hpp"
#include "range.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string_view>
#include <vector>

#include <lodepng.h>

#include <docopt/docopt.h>
#include <spdlog/spdlog.h>
#include <ftxui/component/captured_mouse.hpp>      // for ftxui
#include <ftxui/component/component.hpp>           // for Slider
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <nlohmann/json.hpp>

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `grandrounds`. You can
// modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

namespace grandrounds {

class path_error : public std::runtime_error {
   public:
    path_error(const std::string& message) : std::runtime_error(message) {}
};

class file_error : public std::runtime_error {
   public:
    file_error(const std::string& message) : std::runtime_error(message) {}
};

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
std::string slurp(std::filesystem::path path)
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
    std::vector<std::uint8_t> pixels;
    unsigned int width{};
    unsigned int height{};
};

struct puzzle_data {
    std::string title;
    std::string description;
    std::string author;
    std::string date;
    std::string license;
    std::string wikipedia;
};

struct puzzle {
    unsigned int width{};
    unsigned int height{};
    std::vector<std::uint8_t> nonogram;
    std::vector<std::uint8_t> picture;
    puzzle_data data;
};

puzzle_data load_puzzle_data(std::filesystem::path json_path)
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

loaded_image load_image(std::filesystem::path nonogram_png_path)
{
    loaded_image out;
    const auto error{lodepng::decode(out.pixels, out.width, out.height,
                                     nonogram_png_path.string())};
    if (error) {
        throw file_error{fmt::format("Could not load {}: {} {}",
                                     nonogram_png_path.string(), error,
                                     lodepng_error_text(error))};
    }
    return out;
}

puzzle load_puzzle(std::string_view name)
{
    const auto puzzle_dir{find_puzzles_dir()};
    const auto json_path{puzzle_dir / fmt::format("{}_data.json", name)};
    const auto nonogram_path{puzzle_dir / fmt::format("{}_nonogram.png", name)};
    auto nonogram{load_image(nonogram_path)};
	// Convert the image data to "white = 0, black = 1"
    r::transform(nonogram.pixels, nonogram.pixels.begin(),
                 [](std::uint8_t pixel) -> std::uint8_t { return pixel == 0; });

    puzzle out;
    out.width = nonogram.width;
    out.height = nonogram.height;
    out.nonogram = std::move(nonogram.pixels);
    out.data = load_puzzle_data(json_path);

    return out;
}

void play_puzzle(std::string_view name)
{
    const auto puzzle{load_puzzle(name)};

    std::vector<std::uint8_t> board;
    board.resize(puzzle.nonogram.size());

    std::string quit_text{"Quit"};

    auto screen{ftxui::ScreenInteractive::TerminalOutput()};
    auto quit_button{ftxui::Button(&quit_text, screen.ExitLoopClosure())};

    auto canvas{ftxui::Canvas(100, 100)};

    std::vector<ftxui::Component> all_components;
    all_components.push_back(quit_button);
    auto container = ftxui::Container::Horizontal(all_components);

    auto renderer = ftxui::Renderer(container, [&] {
        ftxui::Element right_panel{
            ftxui::vbox({{ftxui::text("This is the quit button:"),
                          ftxui::separator(), quit_button->Render()}}) |
            ftxui::border | ftxui::xflex_shrink};

        return ftxui::hbox({{ftxui::canvas(canvas) | ftxui::xflex_grow,
                             ftxui::separator(), right_panel}});
    });

    screen.Loop(renderer);
}

}  // namespace

}  // namespace grandrounds

int main(int argc, const char** argv)
{
    try {
        static constexpr auto USAGE =
            R"(grandrounds

    Usage:
          grandrounds
          grandrounds puzzle <NAME>
          grandrounds (-h | --help)
          grandrounds --version
 Options:
          -h --help     Show this screen.
          --version     Show version.
)";

        const std::map<std::string, docopt::value> args{docopt::docopt(
            USAGE, {std::next(argv), std::next(argv, argc)},
            true,  // show help if requested
            fmt::format("{} {}", grandrounds::cmake::project_name,
                        grandrounds::cmake::project_version))};

        if (args.at("puzzle").asBool()) {
            grandrounds::play_puzzle(args.at("<NAME>").asString());
        }
        else {
            // TODO
        }
    }
    catch (const std::exception& e) {
        fmt::print("Unhandled exception in main: {}", e.what());
    }
}
