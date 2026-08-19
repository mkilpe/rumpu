
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <securepath/log/backend/backend.hpp>
#include <securepath/log/backend/file_output.hpp>
#include <securepath/log/log.hpp>

#include "app_options.hpp"
#include "rumpu.hpp"
#include "version.hpp"

#include <securepath/util/command_parser.hpp>

#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Decide GL+GLSL versions and set the matching window hints
static const char* configure_gl_hints() {
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    return "#version 100";
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    return "#version 300 es";
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    return "#version 150";
#else
    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    return "#version 130";
#endif
}

static void setup_imgui(GLFWwindow* window, const char* glsl_version, float main_scale) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

static void render_frame(GLFWwindow* window, securepath::drum::app::rumpu& app) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if(!app.update()) {
        glfwSetWindowShouldClose(window, 1);
    }

    ImGui::Render();

    ImVec4 const clear_color{0.45f, 0.55f, 0.60f, 1.00f};
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

int run_app(securepath::drum::app::app_options options) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    const char* glsl_version = configure_gl_hints();
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Rumpu", nullptr, nullptr);
    if (window == nullptr) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    setup_imgui(window, glsl_version, main_scale);

    securepath::drum::app::rumpu app(std::move(options));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
        } else {
            render_frame(window, app);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int main(int argc, char* argv[]) {
    try {
        securepath::log::backend::add_backend<securepath::log::backend::file_output>("file", "rumpu.log");

        securepath::drum::app::app_options options;
        bool show_help{};
        bool show_version{};
        securepath::command_parser parser;
        parser.add(options.initial_file, "open", "o", "open project file");
        parser.add(show_help, "help", "h", "show this help");
        parser.add(show_version, "version", "v", "show version");
        parser.parse(argc, argv);

        if (show_help) {
            parser.print_help(std::cout);
            return 0;
        }
        if (show_version) {
            std::cout << "Rumpu " << securepath::drum::app::version << std::endl;
            return 0;
        }

        // Support file association: positional argument (e.g. from Windows shell open)
        if (options.initial_file.empty() && argc > 1) {
            std::string arg = argv[argc - 1];
            if (!arg.empty() && arg[0] != '-') {
                options.initial_file = std::move(arg);
            }
        }

        return run_app(std::move(options));
    } catch(std::exception const& ex) {
        std::cout << "exception: " << ex.what() << std::endl;
    }
}
