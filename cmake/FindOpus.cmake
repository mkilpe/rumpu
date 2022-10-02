
find_path(opus_include NAMES opus.h PATH_SUFFIXES opus)
find_library(opus_library NAMES opus)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Opus DEFAULT_MSG opus_library opus_include)

if(Opus_FOUND)
	set(Opus_LIBRARIES ${opus_library})
	set(Opus_INCLUDE_DIRS ${opus_include})
endif()

mark_as_advanced(Opus_INCLUDE_DIRS Opus_LIBRARIES)