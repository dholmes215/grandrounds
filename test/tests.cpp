//
// Copyright (c) 2022 David Holmes (dholmes at dholmes dot us)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "nonogram.hpp"

#define CATCH_CONFIG_NO_WINDOWS_SEH
#include <catch2/catch.hpp>

TEST_CASE("Parse puzzle data from string", "[puzzle_data]")
{
    static constexpr auto json =
        R"({
  "title": "Cottontail on the Trail",
  "description": "This bronze bunny sculpture struct me as very strange when it first appeared suddenly in 2002, but seeing local children playing on it every Easter quickly warmed me to it.",
  "author": "David Holmes",
  "date": "2022",
  "license": "Public Domain",
  "wikipedia": "https://en.wikipedia.org/wiki/Cottontail_on_the_Trail"
})";
	
    grandrounds::puzzle_data data{grandrounds::parse_puzzle_data(json)};
    REQUIRE(data.title == "Cottontail on the Trail");
    REQUIRE(data.description ==
            "This bronze bunny sculpture struct me as very strange when it "
            "first appeared suddenly in 2002, but seeing local children "
            "playing on it every Easter quickly warmed me to it.");
	REQUIRE(data.author == "David Holmes");
	REQUIRE(data.date == "2022");
	REQUIRE(data.license == "Public Domain");
	REQUIRE(data.wikipedia == "https://en.wikipedia.org/wiki/Cottontail_on_the_Trail");
}
