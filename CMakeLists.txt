cmake_minimum_required(VERSION 3.10)

project("rumpu")

set(RUMPU_VERSION_MAJOR 0)
set(RUMPU_VERSION_MINOR 1)
set(RUMPU_VERSION_PATCH 0)
set(RUMPU_VERSION_LABEL "" CACHE STRING "Optional pre-release label, e.g. alpha, beta.1")

set(RUMPU_VERSION "${RUMPU_VERSION_MAJOR}.${RUMPU_VERSION_MINOR}.${RUMPU_VERSION_PATCH}")
if(RUMPU_VERSION_LABEL)
    string(APPEND RUMPU_VERSION "-${RUMPU_VERSION_LABEL}")
endif()

find_package(Git QUIET)
if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE RUMPU_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()
if(NOT RUMPU_GIT_HASH)
    set(RUMPU_GIT_HASH "unknown")
endif()

set(RUMPU_VERSION_FULL "${RUMPU_VERSION}+${RUMPU_GIT_HASH}")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(my_root_directory ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
set(CMAKE_MODULE_PATH ${my_root_directory}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

#find_package(Qt5 5.12.0 COMPONENTS Core Multimedia REQUIRED)
find_package(Opus REQUIRED)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
	message("-- Enabling debug build") 
endif()

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

include_directories(${my_root_directory})
link_directories(${my_root_directory}/lib)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS} -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_LOCAL_FLAGS} -Wall -Wreturn-type -Werror=return-type -fdiagnostics-color=auto -Wno-deprecated-declarations")

add_subdirectory(rumpu)

# base libs
add_subdirectory(securepath)

add_subdirectory(test)
