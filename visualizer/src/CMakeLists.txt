add_executable(HelloWorld hello_world.cpp)
target_link_libraries(HelloWorld SDL2::SDL2main SDL2::SDL2-static)
target_link_libraries(HelloWorld portaudio_static)
