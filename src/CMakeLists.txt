# gets all hpp files in inc
FILE(GLOB ALL_HPPS "${2048-4d-ai_SOURCE_DIR}/inc/*.hpp")

# includes all headers
set(HEADER_LIST ALL_HPPS)

# gets all cpp files in src
FILE(GLOB ALL_CPPS "${2048-4d-ai_SOURCE_DIR}/src/*.cpp")

# creates source library
add_library(src ${ALL_CPPS} ${ALL_HPPS})

# gets include paths
target_include_directories(src PUBLIC ../inc)

# sets c++ version
target_compile_features(src PUBLIC cxx_std_20)