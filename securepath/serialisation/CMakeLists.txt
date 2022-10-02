file(GLOB SRC *.cpp *.hpp codec/*.cpp codec/*.hpp codec/asn_der/*.cpp codec/asn_der/*.hpp)
add_library(serialisation STATIC ${SRC})
target_link_libraries(serialisation util)

add_subdirectory(test)
add_subdirectory(codec/asn_der/test)