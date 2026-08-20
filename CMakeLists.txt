cmake_minimum_required(VERSION 3.25)

project("rumpu")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported; configure with "
        "'cmake --preset default' (or 'cmake -S . -B build'). Remove the "
        "CMakeCache.txt and CMakeFiles/ this attempt just created.")
endif()

set(RUMPU_VERSION_MAJOR 0)
set(RUMPU_VERSION_MINOR 1)
set(RUMPU_VERSION_PATCH 3)
set(RUMPU_VERSION_LABEL "rc.1" CACHE STRING "Optional pre-release label, e.g. alpha, beta.1")

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
message(STATUS "Rumpu version: ${RUMPU_VERSION_FULL}")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fail with a clear message instead of hundreds of C++26 errors
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    message(FATAL_ERROR "GCC ${CMAKE_CXX_COMPILER_VERSION} is too old; C++26 support requires GCC 14 or newer")
endif()

option(RUMPU_SANITIZE "Build with address and undefined-behaviour sanitizers" OFF)
if(RUMPU_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

enable_testing()

get_filename_component(my_root_directory ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
set(CMAKE_MODULE_PATH ${my_root_directory}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# for clangd and other tooling; a compile_commands.json symlink at the source
# root points into the build directory
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MINGW)
    # mingw libstdc++ ships std::print's terminal support in a separate archive
    link_libraries(stdc++exp)
endif()

if(WIN32)
    include(fetch-deps)
else()
    # provides the imported 'glfw' target; a missing dev package fails here
    # with a clear message instead of at link time with a bare -lglfw
    find_package(glfw3 3.3 REQUIRED)
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
	message("-- Enabling debug build") 
endif()

if(UNIX)
    set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)

include_directories(${my_root_directory})
link_directories(${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS} -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS} -O2")
# -Wno-missing-field-initializers: partially initialised aggregates (rest value-initialised) are idiomatic here
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_LOCAL_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wreturn-type -Werror=return-type -fdiagnostics-color=auto -Wno-deprecated-declarations")

# single test framework for all suites (securepath's test_frame and test_rumpu)
add_subdirectory(submodules/catch2 ${CMAKE_BINARY_DIR}/catch2)

add_subdirectory(rumpu)

# base libs
add_subdirectory(securepath)

add_subdirectory(tools)

add_subdirectory(test)

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_PACKAGE_NAME "Rumpu")
    set(CPACK_PACKAGE_VENDOR "Secure Path Oy")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rumpu Drum Machine")
    set(CPACK_PACKAGE_VERSION_MAJOR ${RUMPU_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${RUMPU_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${RUMPU_VERSION_PATCH})
    # include the pre-release label in the installer name and version
    set(CPACK_PACKAGE_VERSION ${RUMPU_VERSION})
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
