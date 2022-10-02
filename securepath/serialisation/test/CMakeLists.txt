file(GLOB SRC *.cpp *.hpp)
add_executable(test_serialisation ${SRC})
target_link_libraries(test_serialisation serialisation test_frame)