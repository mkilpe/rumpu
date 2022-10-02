file(GLOB SRC *.cpp *.hpp)
add_executable(test_asn_der ${SRC})
target_link_libraries(test_asn_der serialisation util test_frame log)