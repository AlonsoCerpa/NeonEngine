#pragma once

#include <mutex>
#include <imgui.h>

class NeonEngine;
class Rendering;
struct GLFWwindow;

class Input {
public:
    static Input* get_instance();
    //void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void initialize();
    void process_viewport_input();

    Input(Input& other) = delete;
    void operator=(const Input&) = delete;

    ImVec2 last_mouse_pos_selecting;
    ImVec2 last_mouse_pos_transforming;
    bool transforming_selected_object;

private:
    Input();
    ~Input();

    void mouse_rotate_camera();

    NeonEngine* neon_engine;
    Rendering* rendering;

    static Input* instance;
    static std::mutex input_mutex;
};