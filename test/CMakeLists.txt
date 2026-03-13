add_subdirectory(${CMAKE_SOURCE_DIR}/submodules/catch2 ${CMAKE_BINARY_DIR}/catch2)

add_executable(test_rumpu
	test_drum_sample.cpp
	test_mixer.cpp
	test_timing.cpp
	test_wav.cpp
)

target_compile_definitions(test_rumpu PRIVATE
	TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(test_rumpu PRIVATE
	Catch2::Catch2WithMain
	rumpu_core
	audio_util
	audio
	util
	log
)
