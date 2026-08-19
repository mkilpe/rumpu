file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp)
add_executable(test_util ${SRC})
target_link_libraries(test_util util test_frame log)
add_test(NAME test_util COMMAND test_util)
