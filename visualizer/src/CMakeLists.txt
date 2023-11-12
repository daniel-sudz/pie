add_executable(HelloWorld hello_world.cpp)
target_link_libraries(HelloWorld SDL2::SDL2main SDL2::SDL2-static)
target_link_libraries(HelloWorld portaudio_static)


add_executable(DemoPlayer bin/demo_player.cpp)
target_link_libraries(DemoPlayer portaudio_static)