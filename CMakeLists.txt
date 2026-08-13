cmake_minimum_required(VERSION 3.10)

project("rumpu")

set(RUMPU_VERSION_MAJOR 0)
set(RUMPU_VERSION_MINOR 1)
set(RUMPU_VERSION_PATCH 2)
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

enable_testing()

get_filename_component(my_root_directory ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
set(CMAKE_MODULE_PATH ${my_root_directory}/cmake)
if(CMAKE_CROSSCOMPILING)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif()

#find_package(Qt5 5.12.0 COMPONENTS Core Multimedia REQUIRED)
if(WIN32)
    include(fetch-deps)
endif()

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
	message("-- Enabling debug build") 
endif()

if(UNIX)
    set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)

include_directories(${my_root_directory})
link_directories(${my_root_directory}/lib)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS} -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_LOCAL_FLAGS} -Wall -Wreturn-type -Werror=return-type -fdiagnostics-color=auto -Wno-deprecated-declarations")

add_subdirectory(rumpu)

# base libs
add_subdirectory(securepath)

add_subdirectory(tools)

add_subdirectory(test)

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_PACKAGE_NAME "Rumpu")
    set(CPACK_PACKAGE_VENDOR "Humanity")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rumpu Drum Machine")
    set(CPACK_PACKAGE_VERSION_MAJOR ${RUMPU_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${RUMPU_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${RUMPU_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Rumpu")
    set(CPACK_PACKAGE_EXECUTABLES "rumpu" "Rumpu")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

    # NSIS-specific
    set(CPACK_NSIS_DISPLAY_NAME "Rumpu")
    set(CPACK_NSIS_PACKAGE_NAME "Rumpu")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH OFF)

    # Start Menu shortcut to the bundled user manual
    set(CPACK_NSIS_MENU_LINKS "Rumpu Manual.html" "Rumpu Manual")

    # .spd file association
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        WriteRegStr HKCR '.spd' '' 'RumpuProject'
        WriteRegStr HKCR 'RumpuProject' '' 'Rumpu Project File'
        WriteRegStr HKCR 'RumpuProject\\\\shell\\\\open\\\\command' '' '\\\"$INSTDIR\\\\bin\\\\rumpu.exe\\\" \\\"%1\\\"'
        System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
    ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        DeleteRegKey HKCR '.spd'
        DeleteRegKey HKCR 'RumpuProject'
        System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
    ")

    include(CPack)
endif()
