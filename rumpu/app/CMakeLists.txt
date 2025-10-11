file(GLOB SRC *.cpp *.hpp)
file(GLOB IMGUI_SRC ../submodules/imgui/*.cpp ../submodules/imgui/*.h)
file(GLOB IMGUI_BACKEND_SRC ../submodules/imgui/backends/imgui_impl_glfw.cpp ../submodules/imgui/backends/imgui_impl_opengl3.cpp)
file(GLOB IMPLOT_SRC ../submodules/implot/*.cpp ../submodules/implot/*.h)

file(GLOB USAGE_SRC usage_source/*.hpp usage_source/homewizard/*.cpp usage_source/homewizard/*.hpp)
file(GLOB PRICE_SRC price_source/*.hpp price_source/sahkohinta-api/*.cpp price_source/sahkohinta-api/*.hpp)
file(GLOB UTIL_SRC util/*.cpp util/*.hpp util/event_system/*.cpp util/event_system/*.hpp)
file(GLOB VIEW_SRC view/*.cpp view/*.hpp)

include_directories(../submodules/imgui)
include_directories(../submodules/implot)

add_executable(energu ${SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC} ${IMPLOT_SRC} ${DATA_SRC} ${UTIL_SRC} ${VIEW_SRC} ${USAGE_SRC} ${PRICE_SRC})
target_link_libraries(energu glfw OpenGL::GL)
