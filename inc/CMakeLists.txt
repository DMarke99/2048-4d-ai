add_library(inc board.hpp game.hpp trans_table.hpp)

target_include_directories(inc PUBLIC "$(CMAKE_CURRENT_SOURCE_DIR)")