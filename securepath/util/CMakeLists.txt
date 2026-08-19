file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp)
add_library(util STATIC ${SRC})
target_link_libraries(util log)

add_subdirectory(test)