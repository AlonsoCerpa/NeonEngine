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

    int window_viewport_width, window_viewport_height;
    int texture_viewport_width, texture_viewport_height;;
    int texture_viewport_reduce_width_px, texture_viewport_reduce_height_px;
    bool first_time_viewport_fbo;
    ImVec2 viewport_texture_pos;

private:
    UserInterface();
    ~UserInterface();

    void set_ui_style();

    float passed_time_seconds;
    float frames_per_second_ui;
    NeonEngine* neon_engine;
    Input* input;
    Rendering* rendering;

    static UserInterface* instance;
    static std::mutex user_interface_mutex;
};