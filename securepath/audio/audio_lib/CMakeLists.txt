
if(NOT DEFINED sound_backend)
	if(WIN32)
		set(sound_backend dsound)
	elseif(UNIX)
		if(${ANDROID})
			message("-- no audio backend for android")
		else()
			set(sound_backend alsa)
		endif()
	endif()
endif()

file(GLOB SRC *.cpp *.hpp)

include_directories(${Opus_INCLUDE_DIRS})

add_library(audio STATIC ${SRC})
target_link_libraries(audio opus)

if(DEFINED sound_backend)
	include(${sound_backend}/CMakeLists.txt)
endif()
