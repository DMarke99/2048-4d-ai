if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto")
set(EXECUTABLE_OUTPUT_PATH "${2048-4d-ai_SOURCE_DIR}/bin")

add_executable(play-ai-game src/play-ai-game.cpp)
target_compile_features(play-ai-game PRIVATE cxx_std_14)
target_link_libraries(play-ai-game PRIVATE src)

add_executable(play-mcts-game src/play-mcts-game.cpp)
target_compile_features(play-mcts-game PRIVATE cxx_std_14)
target_link_libraries(play-mcts-game PRIVATE src)

add_executable(test-game-params src/test-game-params.cpp)
target_compile_features(test-game-params PRIVATE cxx_std_14)
target_link_libraries(test-game-params PRIVATE src)

add_executable(run-many-games src/run-many-games.cpp)
target_compile_features(run-many-games PRIVATE cxx_std_14)
target_link_libraries(run-many-games PRIVATE src)