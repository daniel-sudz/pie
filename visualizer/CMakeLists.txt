cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS)

project(Visualizer LANGUAGES CXX)

# link std::atomic properly based on platform
if(APPLE)
    add_link_options(-march=native)
elseif(UNIX)
    add_link_options( -march=native -latomic)
endif()

# enable debug mode
add_compile_options(-fsanitize=address -g)
add_link_options(-fsanitize=address -g)


add_subdirectory(thirdparty/portaudio)
add_subdirectory(thirdparty/libserial)
add_subdirectory(src)
