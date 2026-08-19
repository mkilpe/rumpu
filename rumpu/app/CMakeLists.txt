file(GLOB SRC CONFIGURE_DEPENDS *.cpp *.hpp)

file(READ "${CMAKE_SOURCE_DIR}/LICENSE" LICENSE_TEXT)
configure_file(license.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/license.hpp" @ONLY)
configure_file(version.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/version.hpp" @ONLY)

file(GLOB IMGUI_SRC CONFIGURE_DEPENDS ../../submodules/imgui/*.cpp ../../submodules/imgui/*.h ../../submodules/imgui-knobs/*.cpp ../../submodules/imgui-knobs/*.h)
file(GLOB IMGUI_BACKEND_SRC CONFIGURE_DEPENDS ../../submodules/imgui/backends/imgui_impl_glfw.cpp ../../submodules/imgui/backends/imgui_impl_opengl3.cpp)
set(IMGUI_STDLIB_SRC ../../submodules/imgui/misc/cpp/imgui_stdlib.cpp ../../submodules/imgui/misc/cpp/imgui_stdlib.h)

include_directories(../../submodules/imgui)
include_directories(../../submodules/imgui/misc/cpp)
include_directories(../../submodules/imgui-knobs)

if(UNIX AND NOT APPLE)
    list(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/native_file_dialog_windows.cpp)
    list(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/mingw_print_stubs.cpp)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
elseif(WIN32)
    list(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/native_file_dialog_linux.cpp)
    if(NOT MINGW)
        list(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/mingw_print_stubs.cpp)
    endif()
endif()

add_executable(rumpu ${SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC} ${IMGUI_STDLIB_SRC})
target_include_directories(rumpu PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(rumpu rumpu_core event_system glfw OpenGL::GL)

if(UNIX AND NOT APPLE)
    target_include_directories(rumpu PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(rumpu ${GTK3_LIBRARIES})
elseif(WIN32)
    target_link_libraries(rumpu comdlg32 ole32)
endif()

install(TARGETS rumpu RUNTIME DESTINATION bin)

if(WIN32 AND MINGW)
    # Bundle MinGW runtime DLLs
    set(MINGW_DLLS
        libstdc++-6.dll
        libgcc_s_seh-1.dll
        libwinpthread-1.dll
    )
    foreach(dll ${MINGW_DLLS})
        find_file(${dll}_PATH ${dll} PATHS ${CMAKE_FIND_ROOT_PATH}/bin)
        if(${dll}_PATH)
            install(FILES ${${dll}_PATH} DESTINATION bin)
            # keep the copies next to the built exes fresh: stale ones shadow
            # the toolchain's (exe dir wins the DLL search) and crash on the
            # first exception after a toolchain update
            add_custom_command(TARGET rumpu POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${${dll}_PATH}" "$<TARGET_FILE_DIR:rumpu>")
        endif()
    endforeach()
endif()
