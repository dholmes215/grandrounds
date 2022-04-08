# Disabled lines are fetched via FetchContent instead of Conan and do not need find_package
find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)
#find_package(Microsoft.GSL)
find_package(nlohmann_json)
find_package(lodepng REQUIRED)
#find_package(range-v3 REQUIRED)

add_executable(grandrounds main.cpp "grid.hpp" "range.hpp" "nonogram_component.hpp" "nonogram.hpp" "nonogram.cpp" "nonogram_component.cpp")
target_link_libraries(
  grandrounds
  PRIVATE project_options
  project_warnings
  docopt::docopt
  fmt::fmt
  lodepng::lodepng
  Microsoft.GSL::GSL
  nlohmann_json::nlohmann_json
  spdlog::spdlog)

target_link_system_libraries(
  grandrounds
  PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
  range-v3::range-v3)

target_include_directories(grandrounds PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
