project(stb)

add_library(stb src/stb_image.cpp)
target_include_directories(stb PUBLIC include)