if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

set(EXECUTABLE_OUTPUT_PATH "${2048-4d-ai_SOURCE_DIR}/bin")
add_executable(2048-4d-ai src/ai-game.cpp)
target_compile_features(2048-4d-ai PRIVATE cxx_std_14)
target_link_libraries(2048-4d-ai PRIVATE src)