
if(NOT DEFINED sound_backend)
	if(WIN32)
		message("-- using DirectSound")
		set(sound_backend dsound)
	elseif(UNIX)
		if(${ANDROID})
			message("-- no audio backend for android")
		else()
			message("-- using Alsa")
			set(sound_backend alsa)
		endif()
	endif()
endif()

file(GLOB SRC *.cpp *.hpp)
add_library(audio STATIC ${SRC})

if(DEFINED sound_backend)
	include(${sound_backend}/CMakeLists.txt)
endif()

include_directories(${Opus_INCLUDE_DIRS})
target_link_libraries(audio opus)