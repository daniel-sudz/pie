cmake_minimum_required (VERSION 3.8)
include(FetchContent)

project(HelloWorld LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

include_directories("/opt/homebrew/Cellar/boost/1.83.0/include")


# Compile the SDL2 library
set (CMAKE_C_FLAGS "-Wno-deprecated-declarations")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.26.5
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")

add_subdirectory(thirdparty/portaudio)
add_subdirectory(thirdparty/libserial)
add_subdirectory(src)
