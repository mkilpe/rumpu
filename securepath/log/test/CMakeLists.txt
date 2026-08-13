file(GLOB SRC *.cpp *.hpp)
add_executable(test_log ${SRC})
target_link_libraries(test_log log)
add_test(NAME test_log COMMAND test_log)
