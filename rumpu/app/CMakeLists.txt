file(GLOB SRC *.cpp *.hpp)
file(GLOB IMGUI_SRC ../../submodules/imgui/*.cpp ../../submodules/imgui/*.h ../../submodules/imgui-knobs/*.cpp ../../submodules/imgui-knobs/*.h)
file(GLOB IMGUI_BACKEND_SRC ../../submodules/imgui/backends/imgui_impl_glfw.cpp ../../submodules/imgui/backends/imgui_impl_opengl3.cpp)

include_directories(../../submodules/imgui)
include_directories(../../submodules/imgui-knobs)

add_executable(rumpu ${SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})
target_link_libraries(rumpu rumpu_core event_system glfw OpenGL::GL)