#pragma once

#include "imgui_extension.h"

#include <imgui.h>
#include <mutex>

class NeonEngine;
class Input;
class Rendering;

class UserInterface {
public:
    static UserInterface* get_instance();
    static void glfw_error_callback(int error, const char* description);

    UserInterface(UserInterface& other) = delete;
    void operator=(const UserInterface&) = delete;

    void initialize();
    void setup_imgui();
    void render_ui();
    void clean_imgui();

    int viewport_width, viewport_height;
    bool first_time_viewport_fbo;

private:
    UserInterface();
    ~UserInterface();

    void set_ui_style();

    NeonEngine* neon_engine;
    Input* input;
    Rendering* rendering;

    static UserInterface* instance;
    static std::mutex user_interface_mutex;
};