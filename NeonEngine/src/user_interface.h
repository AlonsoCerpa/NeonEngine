#pragma once

#include "imgui_extension.h"

#include <imgui.h>
#include <mutex>

class Input;
struct GLFWwindow;

class UserInterface {
public:
    static UserInterface* get_instance();
    static void glfw_error_callback(int error, const char* description);

    UserInterface(UserInterface& other) = delete;
    void operator=(const UserInterface&) = delete;

    void initialize();
    int setup_glfw();
    int setup_glad();
    void setup_imgui();
    void render_ui();
    void clean_gflw();
    void clean_imgui();

    GLFWwindow* window;
    ImVec4 clear_color;
    int window_width;
    int window_height;

private:
    UserInterface();
    ~UserInterface();

    void set_ui_style();

    Input* input;
    int glfw_major_version;
    int glfw_minor_version;
    const char* glsl_version;
    const char* window_title;

    static UserInterface* instance;
    static std::mutex user_interface_mutex;
};