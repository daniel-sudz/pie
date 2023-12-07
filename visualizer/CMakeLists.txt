cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_COMPILER g++)

project(HelloWorld LANGUAGES CXX)

if(APPLE)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address -march=native)
elseif(UNIX)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address -march=native -latomic)
endif()



add_subdirectory(thirdparty/portaudio)
add_subdirectory(thirdparty/libserial)
add_subdirectory(src)
