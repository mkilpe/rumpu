file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp backend/*.cpp backend/*.hpp detail/*.cpp detail/*.hpp)
add_library(log STATIC ${SRC})

add_subdirectory(test)