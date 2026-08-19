file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp)
add_executable(test_asn_der ${SRC})
target_link_libraries(test_asn_der serialisation util test_frame log)
add_test(NAME test_asn_der COMMAND test_asn_der)
