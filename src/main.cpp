//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "grid.hpp"
#include "range.hpp"

#include <gsl/narrow>

#include <docopt/docopt.h>
#include <lodepng.h>
#include <spdlog/spdlog.h>
#include <ftxui/component/captured_mouse.hpp>      // for ftxui
#include <ftxui/component/component.hpp>           // for Slider
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string_view>
#include <vector>

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `grandrounds`. You can
// modify the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

namespace grandrounds {

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

struct puzzle_data {
    std::string title;
    std::string description;
    std::string author;
    std::string date;
    std::string license;
    std::string wikipedia;
};

struct color {
    // TODO
};

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
    // FIXME: constructor
    explicit nonogram_puzzle(std::string_view name);

    board_coords dimensions;
    std::vector<std::uint8_t> nonogram;
    std::vector<color> picture;
    puzzle_data data;
    std::vector<std::vector<std::uint8_t>> row_hints;
    std::vector<std::vector<std::uint8_t>> col_hints;
    int row_hints_max{0};
    int col_hints_max{0};
};

struct nonogram_game {
    std::shared_ptr<nonogram_puzzle> puzzle;
    std::vector<uint8_t> board;
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

class nonogram_component : public ftxui::ComponentBase {
   public:
    explicit nonogram_component(std::shared_ptr<nonogram_game> game)
        : game_{std::move(game)},
          board_position_{game_->puzzle->row_hints_max * 3 + 1,
                          game_->puzzle->col_hints_max + 1}
    {
    }

    ftxui::Element Render() override { return ftxui::canvas(draw_board()); }

    bool OnEvent(ftxui::Event event) override
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
            if (in_range) {
                if (event.mouse().motion == ftxui::Mouse::Pressed) {
                    const auto board_idx{static_cast<std::size_t>(
                        selected_.y * width + selected_.x)};
                    if (event.mouse().button == ftxui::Mouse::Left) {
                        game_->board[board_idx] = 1;
                    }
                    if (event.mouse().button == ftxui::Mouse::Right) {
                        game_->board[board_idx] = 0;
                    }
                    // TODO: Replace uint8 with enum and support middle-click
                }
            }
            else {
                selected_ = {-1, -1};
            }
        }

        return false;
    }

    void Solve() { game_->board = game_->puzzle->nonogram; }

   private:
    const ftxui::Color black{0, 0, 0};
    const ftxui::Color almost_black{32, 32, 32};
    const ftxui::Color black_highlight{32, 32, 64};
    const ftxui::Color white{255, 255, 255};
    const ftxui::Color white_highlight{223, 223, 255};

    static void draw_rect(ftxui::Canvas& canvas,
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

    [[nodiscard]] ftxui::Color square_color(board_coords square) const noexcept
    {
        const auto& board{game_->board};
        const int width{game_->puzzle->dimensions.x};
        if (board[gsl::narrow<std::size_t>(square.y * width + square.x)] != 0) {
            if (selected_.x == square.x || selected_.y == square.y) {
                return black_highlight;
            }
            else {
                return almost_black;
            }
        }
        else {
            if (selected_.x == square.x || selected_.y == square.y) {
                return white_highlight;
            }
            else {
                return white;
            }
        }
    }

    [[nodiscard]] ftxui::Canvas draw_board() const
    {
        // FIXME: reduce complexity
        const auto& puzzle{*game_->puzzle};
        const int width{game_->puzzle->dimensions.x};
        const int height{game_->puzzle->dimensions.y};

        ftxui::Canvas out{(width + board_position_.x) * 4,
                          (height + board_position_.y) * 4};

        // Draw board
        for (const auto [x, y] :
             rv::cartesian_product(rv::ints(0, width), rv::ints(0, height))) {
            // TODO: extract function to convert coordinates
            draw_rect(out, 4 * x + 2 * board_position_.x,
                      4 * (y + board_position_.y), 4, 4, true,
                      square_color({x, y}));
        }

        const std::function default_stylizer{[=](ftxui::Pixel& p) {
            p.background_color = black;
            p.foreground_color = white;
        }};

        const std::function highlight_stylizer{[=](ftxui::Pixel& p) {
            p.background_color = white_highlight;
            p.foreground_color = black;
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
                    (board_position_.x - (3 * (gsl::narrow<int>(i) + 1)) - 1) *
                    2};
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
            for (auto [i, hint] :
                 this_col_hints | rv::reverse | rv::enumerate) {
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

    std::shared_ptr<nonogram_game> game_;  // State of the game in progress
    board_coords selected_{-1, -1};  // Currently-selected square on the board
    term_coords board_position_;     // Terminal coordinates where the top-left
                                     // character of the board will be drawn
};

void play_puzzle(std::string_view name)
{
    int mouse_x{0};
    int mouse_y{0};
    int frame{0};

    auto game{std::make_shared<nonogram_game>()};
    // TODO: Move this to nonogram_game constructor or static factory or
    // something
    game->puzzle = std::make_shared<nonogram_puzzle>(name);
    game->board.resize(game->puzzle->nonogram.size());

    const std::string solve_text{"Solve"};
    const std::string quit_text{"Quit"};

    auto screen{ftxui::ScreenInteractive::TerminalOutput()};

    auto puzzle_component{std::make_shared<nonogram_component>(game)};
    auto solve_button{
        ftxui::Button(&solve_text, [&] { puzzle_component->Solve(); })};
    auto quit_button{ftxui::Button(&quit_text, screen.ExitLoopClosure())};
    auto right_container{
        ftxui::Container::Vertical({solve_button, quit_button})};

    std::vector<ftxui::Component> all_components;
    all_components.push_back(puzzle_component);

    auto right_panel{ftxui::Renderer(right_container, [&] {
        return ftxui::vbox(
            {{ftxui::text(fmt::format("Mouse: {},{}", mouse_x, mouse_y)),
              ftxui::text(fmt::format("Frame: {}", frame++)),
              solve_button->Render(), quit_button->Render()}});
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
