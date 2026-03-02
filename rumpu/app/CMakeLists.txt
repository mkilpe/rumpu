file(GLOB SRC *.cpp *.hpp)

file(READ "${CMAKE_SOURCE_DIR}/LICENSE" LICENSE_TEXT)
configure_file(license.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/license.hpp" @ONLY)

file(GLOB IMGUI_SRC ../../submodules/imgui/*.cpp ../../submodules/imgui/*.h ../../submodules/imgui-knobs/*.cpp ../../submodules/imgui-knobs/*.h)
file(GLOB IMGUI_BACKEND_SRC ../../submodules/imgui/backends/imgui_impl_glfw.cpp ../../submodules/imgui/backends/imgui_impl_opengl3.cpp)

include_directories(../../submodules/imgui)
include_directories(../../submodules/imgui-knobs)

if(UNIX AND NOT APPLE)
    list(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/native_file_dialog_windows.cpp)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
elseif(WIN32)
    list(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/native_file_dialog_linux.cpp)
endif()

add_executable(rumpu ${SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})
target_include_directories(rumpu PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(rumpu rumpu_core event_system glfw OpenGL::GL)

if(UNIX AND NOT APPLE)
    target_include_directories(rumpu PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(rumpu ${GTK3_LIBRARIES})
elseif(WIN32)
    target_link_libraries(rumpu comdlg32 ole32)
endif()
