add_executable(DemoPlayer bin/demo_player.cpp)
target_link_libraries(DemoPlayer PortAudio)

add_executable(DemoSerialReciever bin/demo_serial_reciever.cpp)
target_link_libraries(DemoSerialReciever PortAudio libserial_static)

add_executable(EtchSketch bin/etch_a_sketch.cpp)
target_link_libraries(EtchSketch PortAudio libserial_static)

add_executable(TestSegFault bin/test_debug_segfault.cpp)
target_link_libraries(TestSegFault PortAudio libserial_static)
