file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp)
add_library(rumpu_core STATIC ${SRC})
# PUBLIC: the core headers include securepath headers (player.hpp, section.hpp,
# track.hpp, export.hpp, rand_hit.hpp), so consumers such as generate_demo
# need the include paths as well as the libraries
target_link_libraries(rumpu_core PUBLIC
	securepath::audio_util
	securepath::audio
	securepath::event_system
	securepath::serialisation
	securepath::util
	securepath::log
)
