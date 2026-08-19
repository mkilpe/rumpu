add_executable(test_rumpu
	test_async_dialog_result.cpp
	test_audio_buffer.cpp
	test_audio_data.cpp
	test_drum_sample.cpp
	test_export.cpp
	test_mixer.cpp
	test_player_cursor.cpp
	test_song.cpp
	test_timing.cpp
	test_ui_task.cpp
	test_undo_manager.cpp
	test_wav.cpp
)

if(UNIX)
	target_sources(test_rumpu PRIVATE test_alsa.cpp)
endif()

if(WIN32)
	target_sources(test_rumpu PRIVATE test_dsound.cpp)
endif()

if(WIN32 AND MINGW)
	# same runtime-DLL freshness as the app target (see rumpu/app/CMakeLists.txt)
	foreach(dll libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
		find_file(${dll}_PATH ${dll} PATHS ${CMAKE_FIND_ROOT_PATH}/bin)
		if(${dll}_PATH)
			add_custom_command(TARGET test_rumpu POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					"${${dll}_PATH}" "$<TARGET_FILE_DIR:test_rumpu>")
		endif()
	endforeach()
endif()

# all test-generated files go here, never next to the committed fixtures
# (with an in-source build the binary dir IS the source dir, hence a subdir)
set(TEST_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/output")
file(MAKE_DIRECTORY "${TEST_OUTPUT_DIR}")

target_compile_definitions(test_rumpu PRIVATE
	# committed input fixtures only — tests must never write here
	TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
	TEST_OUT_DIR="${TEST_OUTPUT_DIR}"
)

target_link_libraries(test_rumpu PRIVATE
	Catch2::Catch2WithMain
	rumpu_core
	event_system
	audio_util
	audio
	util
	log
)

add_test(NAME test_rumpu COMMAND test_rumpu)
