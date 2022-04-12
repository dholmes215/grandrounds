# Disabled lines are fetched via FetchContent instead of Conan and do not need find_package
find_package(fmt REQUIRED)
#find_package(Microsoft.GSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(lodepng REQUIRED)
#find_package(range-v3 REQUIRED)

add_executable(grandrounds main.cpp)
target_link_libraries(
  grandrounds
  PRIVATE project_options
  project_warnings)

target_link_system_libraries(
  grandrounds
  PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
  range-v3::range-v3)

target_include_directories(grandrounds PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
