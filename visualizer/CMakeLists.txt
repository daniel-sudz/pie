cmake_minimum_required (VERSION 3.8)
include(FetchContent)

project(HelloWorld LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address -march=native)

add_subdirectory(thirdparty/portaudio)
add_subdirectory(thirdparty/libserial)
add_subdirectory(src)
