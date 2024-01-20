file(GLOB SRC *.cpp *.hpp)
add_library(rumpu_core STATIC ${SRC})
target_link_libraries(rumpu_core PRIVATE audio_util PUBLIC Qt5::Core Qt5::Multimedia)
