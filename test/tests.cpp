//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "file.hpp"
#include "nonogram.hpp"

#include <gsl/narrow>

#include <cstring>
#include <sstream>

#define CATCH_CONFIG_NO_WINDOWS_SEH
#include <catch2/catch.hpp>

TEST_CASE("Parse puzzle data from string", "[nonogram]")
{
    static constexpr auto json{
        R"({
  "title": "Cottontail on the Trail",
  "description": "This bronze bunny sculpture struct me as very strange when it first appeared suddenly in 2002, but seeing local children playing on it every Easter quickly warmed me to it.",
  "author": "David Holmes",
  "date": "2022",
  "license": "Public Domain",
  "wikipedia": "https://en.wikipedia.org/wiki/Cottontail_on_the_Trail"
})"};

    grandrounds::puzzle_data data{grandrounds::parse_puzzle_data(json)};
    REQUIRE(data.title == "Cottontail on the Trail");
    REQUIRE(data.description ==
            "This bronze bunny sculpture struct me as very strange when it "
            "first appeared suddenly in 2002, but seeing local children "
            "playing on it every Easter quickly warmed me to it.");
    REQUIRE(data.author == "David Holmes");
    REQUIRE(data.date == "2022");
    REQUIRE(data.license == "Public Domain");
    REQUIRE(data.wikipedia ==
            "https://en.wikipedia.org/wiki/Cottontail_on_the_Trail");
}

// Forward-declare this overload of slurp which is omitted from file.hpp
namespace grandrounds {
std::string slurp(std::istream& stream);
}  // namespace grandrounds

TEST_CASE("Read an entire istream into a string", "[file]")
{
    static constexpr auto empty{""};
    std::stringstream ss_empty{empty};
    REQUIRE(grandrounds::slurp(ss_empty) == empty);
    REQUIRE(gsl::narrow<std::size_t>(ss_empty.tellg()) == std::strlen(empty));
    REQUIRE(!ss_empty.fail());

    static constexpr auto lorem{
        R"(    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do
eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut
enim ad minim veniam, quis nostrud exercitation ullamco
laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu
fugiat nulla pariatur. Excepteur sint occaecat cupidatat non
proident, sunt in culpa qui officia deserunt mollit anim id est
laborum.)"};
    std::stringstream ss_lorem{lorem};
    REQUIRE(grandrounds::slurp(ss_lorem) == lorem);
    REQUIRE(gsl::narrow<std::size_t>(ss_lorem.tellg()) == std::strlen(lorem));
    REQUIRE(!ss_lorem.fail());
}