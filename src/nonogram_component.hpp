//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NONOGRAM_COMPONENT_HPP
#define NONOGRAM_COMPONENT_HPP

#include "nonogram.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

namespace grandrounds {

class nonogram_component : public ftxui::ComponentBase {
   public:
    explicit nonogram_component(std::shared_ptr<nonogram_game> game);

    ftxui::Element Render() override;

    bool OnEvent(ftxui::Event event) override;

    void Solve();

   private:
    static void draw_rect(ftxui::Canvas& canvas,
                          int x,
                          int y,
                          int width,
                          int height,
                          bool value,
                          ftxui::Color color);

    [[nodiscard]] ftxui::Color square_color(board_coords square) const noexcept;
	
    [[nodiscard]] ftxui::Canvas draw_photo() const;
    [[nodiscard]] ftxui::Canvas draw_board() const;

    std::shared_ptr<nonogram_game> game_;  // State of the game in progress
    board_coords selected_{-1, -1};  // Currently-selected square on the board
    term_coords board_position_;     // Terminal coordinates where the top-left
                                     // character of the board will be drawn
	bool solved_{false};
};

}  // namespace grandrounds

#endif  // NONOGRAM_COMPONENT_HPP
