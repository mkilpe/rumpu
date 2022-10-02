file(GLOB SRC *.cpp *.hpp)
add_executable(test_util ${SRC})
target_link_libraries(test_util util test_frame log)