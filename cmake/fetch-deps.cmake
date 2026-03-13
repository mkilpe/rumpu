include(FetchContent)

# ----- Opus -----
FetchContent_Declare(opus
    URL https://downloads.xiph.org/releases/opus/opus-1.5.2.tar.gz
)
set(OPUS_BUILD_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_TESTING        OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_PROGRAMS       OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_PKG_CONFIG_MODULE OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(opus)
# FetchContent builds the 'opus' CMake target; no further wiring needed.

# ----- GLFW -----
FetchContent_Declare(glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
# FetchContent builds the 'glfw' CMake target used by rumpu/app.
