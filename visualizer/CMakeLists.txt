cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER g++)

# increase stack size based on platform
if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-stack_size -Wl,0x20000000")
elseif(UNIX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=536870912")
endif()

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
