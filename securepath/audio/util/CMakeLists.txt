file(GLOB SRC *.cpp *.hpp detail/*.cpp detail/*.hpp)
add_library(audio_util STATIC ${SRC})
target_link_libraries(audio_util audio util log)
