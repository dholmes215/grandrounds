# Disabled lines are fetched via FetchContent instead of Conan and do not need find_package
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(lodepng REQUIRED)

# Game library
add_library(game_library game.cpp game.hpp grid.hpp range.hpp nonogram.hpp nonogram.cpp nonogram_ftxui.cpp file.hpp file.cpp)

target_link_libraries(
	game_library
  PRIVATE 
	project_options
	project_warnings
	fmt::fmt
	lodepng::lodepng
	Microsoft.GSL::GSL
	nlohmann_json::nlohmann_json)

target_link_system_libraries(
	game_library
  PRIVATE
	ftxui::screen
	ftxui::dom
	ftxui::component
	range-v3::range-v3)

target_include_directories(
	game_library
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR} 
  PRIVATE 
	"${CMAKE_BINARY_DIR}/configured_files/include")

# Game executable
add_executable(grandrounds main.cpp)
target_link_libraries(
	grandrounds
  PRIVATE 
	project_options 
	project_warnings
	game_library
	fmt::fmt
	Microsoft.GSL::GSL)
target_include_directories(grandrounds PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
