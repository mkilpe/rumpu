file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp)
add_executable(test_log ${SRC})
target_link_libraries(test_log log Catch2::Catch2)
add_test(NAME test_log COMMAND test_log)
