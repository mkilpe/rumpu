file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp)
add_library(test_frame STATIC ${SRC})
target_link_libraries(test_frame PUBLIC Catch2::Catch2)
