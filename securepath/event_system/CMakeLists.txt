file(GLOB SRC *.cpp *.hpp)
add_library(event_system STATIC ${SRC})
#target_link_libraries(event_system util)
